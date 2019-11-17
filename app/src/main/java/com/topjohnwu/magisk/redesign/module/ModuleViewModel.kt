package com.topjohnwu.magisk.redesign.module

import androidx.annotation.WorkerThread
import androidx.databinding.Bindable
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoByNameDao
import com.topjohnwu.magisk.data.database.RepoByUpdatedDao
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.download.RemoteFileService
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.module.Module
import com.topjohnwu.magisk.model.entity.module.Repo
import com.topjohnwu.magisk.model.entity.recycler.InstallModule
import com.topjohnwu.magisk.model.entity.recycler.ModuleItem
import com.topjohnwu.magisk.model.entity.recycler.RepoItem
import com.topjohnwu.magisk.model.entity.recycler.SectionTitle
import com.topjohnwu.magisk.model.events.InstallExternalModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.model.events.dialog.ModuleInstallDialog
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.compat.Queryable
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.tasks.RepoUpdater
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposable
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter
import org.jetbrains.annotations.NotNull
import timber.log.Timber
import kotlin.math.roundToInt

class ModuleViewModel(
    private val repoName: RepoByNameDao,
    private val repoUpdated: RepoByUpdatedDao,
    private val repoUpdater: RepoUpdater
) : CompatViewModel(), Queryable by Queryable.impl(1000) {

    override val queryRunnable = Runnable { query() }

    var query = ""
        @Bindable get
        set(value) {
            if (field == value) return
            field = value
            notifyPropertyChanged(BR.query)
            submitQuery()
            // Yes we do lie about the search being loaded
            searchLoading.value = true
        }

    private var queryJob: Disposable? = null
    val searchLoading = KObservableField(false)
    val itemsSearch = diffListOf<RepoItem>()
    val itemSearchBinding = itemBindingOf<RepoItem> {
        it.bindExtra(BR.viewModel, this)
    }

    val adapter = adapterOf<ComparableRvItem<*>>()
    val items = diffListOf<ComparableRvItem<*>>()
    val itemBinding = itemBindingOf<ComparableRvItem<*>> {
        it.bindExtra(BR.viewModel, this)
    }

    companion object {
        private val sectionRemote =
            SectionTitle(R.string.module_section_remote, R.string.sorting_order)
        private val sectionActive = SectionTitle(
            R.string.module_section_installed,
            R.string.reboot,
            R.drawable.ic_restart
        ).also { it.hasButton.value = false }

        init {
            updateOrderIcon()
        }

        private fun updateOrderIcon() {
            sectionRemote.icon = when (Config.repoOrder) {
                Config.Value.ORDER_NAME -> R.drawable.ic_order_name
                Config.Value.ORDER_DATE -> R.drawable.ic_order_date
                else -> return
            }
        }
    }

    // ---

    private val itemsInstalled
        @WorkerThread get() = items.asSequence()
            .filterIsInstance<ModuleItem>()
            .toList()
    private val itemsRemote
        @WorkerThread get() = items.filterIsInstance<RepoItem>()

    private var remoteJob: Disposable? = null
    private val dao
        get() = when (Config.repoOrder) {
            Config.Value.ORDER_DATE -> repoUpdated
            Config.Value.ORDER_NAME -> repoName
            else -> throw IllegalArgumentException()
        }

    // ---

    init {
        RemoteFileService.reset()
        RemoteFileService.progressBroadcast.observeForever {
            val (progress, subject) = it ?: return@observeForever
            if (subject !is DownloadSubject.Module) {
                return@observeForever
            }
            update(subject.module, progress.times(100).roundToInt())
        }
    }

    // ---

    override fun refresh() = Single.fromCallable { Module.loadModules() }
        .map { it.map { ModuleItem(it) } }
        .map { it.order() }
        .map { build(active = it) }
        .map { it to items.calculateDiff(it) }
        .applyViewModel(this)
        .subscribeK {
            items.update(it.first, it.second)
            if (!items.contains(sectionRemote)) {
                loadRemote()
            } else {
                Single.fromCallable { itemsRemote }
                    .subscribeK { it.ensureUpdateState() }
                    .add()
            }
            moveToState()
        }

    fun loadRemoteImplicit() = let { items.clear(); itemsSearch.clear() }
        .run { downloadRepos() }
        .applyViewModel(this, false)
        .subscribeK { refresh(); submitQuery() }
        .add()

    @Synchronized
    fun loadRemote() {
        // check for existing jobs
        if (remoteJob?.isDisposed?.not() == true) {
            return
        }
        remoteJob = Single.fromCallable { itemsRemote.size }
            .flatMap { loadRemoteInternal(offset = it) }
            .subscribeK(onError = Timber::e) {
                if (!items.contains(sectionRemote)) {
                    items.add(sectionRemote)
                }
                items.addAll(it)
            }
    }

    // ---

    override fun submitQuery() {
        queryHandler.removeCallbacks(queryRunnable)
        queryHandler.postDelayed(queryRunnable, queryDelay)
    }

    private fun queryInternal(query: String, offset: Int): Single<List<RepoItem>> {
        if (query.isBlank()) {
            return Single.just(listOf<RepoItem>())
                .doOnSubscribe { itemsSearch.clear() }
                .subscribeOn(AndroidSchedulers.mainThread())
        }
        return Single.fromCallable { dao.searchRepos(query, offset) }
            .map { it.map { RepoItem(it) } }
            .doOnSuccess { it.ensureUpdateState() }
    }

    private fun query(query: String = this.query, offset: Int = 0) {
        queryJob?.dispose()
        queryJob = queryInternal(query, offset)
            .map { it to itemsSearch.calculateDiff(it) }
            .observeOn(AndroidSchedulers.mainThread())
            .doOnSuccess { searchLoading.value = false }
            .subscribeK { itemsSearch.update(it.first, it.second) }
    }

    @Synchronized
    fun loadMoreQuery() {
        if (queryJob?.isDisposed == false) return
        queryJob = queryInternal(query, itemsSearch.size)
            .subscribeK { itemsSearch.addAll(it) }
    }

    // ---

    private fun loadRemoteInternal(
        offset: Int = 0,
        downloadRepos: Boolean = offset == 0
    ): Single<List<RepoItem>> = Single.fromCallable { dao.getRepos(offset) }
        .map { it.map { RepoItem(it) } }
        .flatMap {
            when {
                // in case we find result empty and offset is initial we need to refresh the repos.
                downloadRepos && it.isEmpty() && offset == 0 -> downloadRepos()
                    .andThen(loadRemoteInternal(downloadRepos = false))
                else -> Single.just(it)
            }
        }
        .doOnSuccess { it.ensureUpdateState() }

    private fun downloadRepos() = Single.just(Unit)
        .flatMap { repoUpdater() }
        .ignoreElement()

    // ---

    @WorkerThread
    private fun List<ModuleItem>.order() = asSequence()
        .sortedBy { it.item.name.toLowerCase(currentLocale) }
        .toList()

    private fun update(repo: Repo, progress: Int) =
        Single.fromCallable { itemsRemote + itemsSearch }
            .map { it.first { it.item.id == repo.id } }
            .subscribeK { it.progress.value = progress }
            .add()

    // ---

    /**
     * Dynamically allocated list of [itemsInstalled]. It might be invalidated any time on any
     * thread hence it needs to be volatile.
     *
     * There might be a state where this field gets assigned `null` whilst being used by another
     * instance of any job, so the list will be immediately reinstated back.
     *
     * ### Note:
     *
     * It is caller's responsibility to invalidate this variable at the end of every job to save
     * memory.
     * */
    @Volatile
    private var cachedItemsInstalled: List<ModuleItem>? = null
        @WorkerThread @NotNull get() = field ?: itemsInstalled.also { field = it }

    private val Repo.isUpdatable: Boolean
        @WorkerThread get() {
            val installed = cachedItemsInstalled!!
                .firstOrNull { it.item.id == id }
                ?: return false
            return installed.item.versionCode < versionCode
        }

    /**
     * Asynchronously updates state of all repo items so the loading speed is not impaired by this
     * seemingly unnecessary operation. Because of the nature of this operation, the "update" status
     * is not guaranteed for all items and can change any time.
     *
     * It is permitted running this function in parallel; it will also attempt to run in parallel
     * by itself to finish the job as quickly as possible.
     *
     * No list manipulations should be done in this method whatsoever! By being heavily parallelized
     * is will inevitably throw exceptions by simultaneously accessing the same list.
     *
     * In order to save time it uses helper [cachedItemsInstalled].
     * */
    private fun List<RepoItem>.ensureUpdateState() = Single.just(this)
        .flattenAsFlowable { it }
        .parallel()
        .map { it to it.item.isUpdatable }
        .sequential()
        .doOnComplete { cachedItemsInstalled = null }
        .subscribeK { it.first.isUpdate.value = it.second }
        .add()

    // ---

    fun moveToState() = Single.fromCallable { itemsInstalled.any { it.isModified } }
        .subscribeK { sectionActive.hasButton.value = it }
        .add()

    fun sectionPressed(item: SectionTitle) = when (item) {
        sectionActive -> reboot() //TODO add reboot picker, regular reboot is not always preferred
        sectionRemote -> {
            Config.repoOrder = when (Config.repoOrder) {
                Config.Value.ORDER_NAME -> Config.Value.ORDER_DATE
                Config.Value.ORDER_DATE -> Config.Value.ORDER_NAME
                else -> Config.Value.ORDER_NAME
            }
            updateOrderIcon()
            Single.fromCallable { itemsRemote }
                .subscribeK {
                    items.removeAll(it)
                    remoteJob?.dispose()
                    loadRemote()
                }.add()
        }
        else -> Unit
    }

    fun downloadPressed(item: RepoItem) = ModuleInstallDialog(item.item).publish()
    fun installPressed() = InstallExternalModuleEvent().publish()
    fun infoPressed(item: RepoItem) = OpenChangelogEvent(item.item).publish()

    // ---

    /** Callable only from worker thread because of expensive list filtering */
    @WorkerThread
    private fun build(
        active: List<ModuleItem> = itemsInstalled,
        remote: List<RepoItem> = itemsRemote
    ) = (active + InstallModule).prependIfNotEmpty { sectionActive } +
            remote.prependIfNotEmpty { sectionRemote }

    private fun <T> List<T>.prependIfNotEmpty(item: () -> T) =
        if (isNotEmpty()) listOf(item()) + this else this

}

fun <T : ComparableRvItem<*>> adapterOf() = object : BindingRecyclerViewAdapter<T>() {
    override fun onBindBinding(
        binding: ViewDataBinding,
        variableId: Int,
        layoutRes: Int,
        position: Int,
        item: T
    ) {
        super.onBindBinding(binding, variableId, layoutRes, position, item)
        item.onBindingBound(binding)
    }
}