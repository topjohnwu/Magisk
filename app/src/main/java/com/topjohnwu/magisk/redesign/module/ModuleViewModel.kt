package com.topjohnwu.magisk.redesign.module

import androidx.annotation.WorkerThread
import androidx.databinding.Bindable
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
import com.topjohnwu.magisk.model.entity.recycler.*
import com.topjohnwu.magisk.model.events.InstallExternalModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.model.events.dialog.ModuleInstallDialog
import com.topjohnwu.magisk.redesign.compat.*
import com.topjohnwu.magisk.tasks.RepoUpdater
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposable
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
        private val sectionRemote = SectionTitle(
            R.string.module_section_remote,
            R.string.sorting_order
        )

        private val sectionUpdate = SectionTitle(
            R.string.module_section_pending,
            R.string.module_section_pending_action,
            R.drawable.ic_update_md2
            // enable with implementation of https://github.com/topjohnwu/Magisk/issues/2036
        ).also { it.hasButton = false }

        private val sectionActive = SectionTitle(
            R.string.module_section_installed,
            R.string.reboot,
            R.drawable.ic_restart
        ).also { it.hasButton = false }

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
        @WorkerThread get() = items.filterIsInstance<ModuleItem>()

    private val itemsUpdatable
        @WorkerThread get() = items.filterIsInstance<RepoItem.Update>()

    private val itemsRemote
        @WorkerThread get() = items.filterIsInstance<RepoItem.Remote>()

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
        .map { build(active = it, updatable = loadUpdates(it)) }
        .map { it to items.calculateDiff(it) }
        .applyViewModel(this)
        .subscribeK {
            items.update(it.first, it.second)
            if (!items.contains(sectionRemote)) {
                loadRemote()
            }
            updateActiveState()
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
        if (!queryHandler.hasCallbacks(queryRunnable)) {
            queryHandler.post(queryRunnable)
        } else {
            queryHandler.removeCallbacks(queryRunnable)
            queryHandler.postDelayed(queryRunnable, queryDelay)
        }
    }

    private fun queryInternal(query: String, offset: Int): Single<List<RepoItem>> {
        if (query.isBlank()) {
            return Single.just(listOf<RepoItem>())
                .doOnSubscribe { itemsSearch.clear() }
                .subscribeOn(AndroidSchedulers.mainThread())
        }
        return Single.fromCallable { dao.searchRepos(query, offset) }
            .map { it.map { RepoItem.Remote(it) } }
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
        .map { it.map { RepoItem.Remote(it) } }
        .flatMap {
            when {
                // in case we find result empty and offset is initial we need to refresh the repos.
                downloadRepos && it.isEmpty() && offset == 0 -> downloadRepos()
                    .andThen(loadRemoteInternal(downloadRepos = false))
                else -> Single.just(it)
            }
        }

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

    @WorkerThread
    private fun loadUpdates(installed: List<ModuleItem>) = installed
        .mapNotNull { dao.getUpdatableRepoById(it.item.id, it.item.versionCode) }
        .map { RepoItem.Update(it) }

    // ---

    fun updateActiveState() = Single.fromCallable { itemsInstalled.any { it.isModified } }
        .subscribeK { sectionActive.hasButton = it }
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
        updatable: List<RepoItem.Update> = itemsUpdatable,
        remote: List<RepoItem.Remote> = itemsRemote
    ) = (active + InstallModule)
        .prependIfNotEmpty { sectionActive }
        .prependIf(Config.coreOnly) { SafeModeNotice } +
            updatable.prependIfNotEmpty { sectionUpdate } +
            remote.prependIfNotEmpty { sectionRemote }

    private fun <T> List<T>.prependIf(condition: Boolean, item: () -> T) =
        if (condition) listOf(item()) + this else this

    private fun <T> List<T>.prependIfNotEmpty(item: () -> T) =
        prependIf(isNotEmpty(), item)

}