package com.topjohnwu.magisk.ui.module

import androidx.annotation.WorkerThread
import androidx.databinding.Bindable
import androidx.databinding.ObservableArrayList
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.download.RemoteFileService
import com.topjohnwu.magisk.core.model.module.Module
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.core.tasks.RepoUpdater
import com.topjohnwu.magisk.data.database.RepoByNameDao
import com.topjohnwu.magisk.data.database.RepoByUpdatedDao
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.addOnListChangedCallback
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.recycler.*
import com.topjohnwu.magisk.model.events.InstallExternalModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.model.events.dialog.ModuleInstallDialog
import com.topjohnwu.magisk.ui.base.*
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import com.topjohnwu.magisk.utils.KObservableField
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposable
import io.reactivex.schedulers.Schedulers
import me.tatarka.bindingcollectionadapter2.collections.MergeObservableList
import timber.log.Timber
import kotlin.math.roundToInt

/*
* The repo fetching behavior should follow these rules:
*
* For the first time the repo list is queried in the app, it should ALWAYS fetch for
* updates. However, this particular fetch should go through RepoUpdater.invoke(false),
* which internally will set ETAGs when doing GET requests to GitHub's API and will
* only update repo DB only if the GitHub API shows that something is changed remotely.
*
* When a user explicitly requests a full DB refresh, it should ALWAYS do a full force
* refresh, which in code can be done with RepoUpdater.invoke(true). This will update
* every single repo's information regardless whether GitHub's API shows if there is
* anything changed or not.
* */

class ModuleViewModel(
    private val repoName: RepoByNameDao,
    private val repoUpdated: RepoByUpdatedDao,
    private val repoUpdater: RepoUpdater
) : BaseViewModel(), Queryable by Queryable.impl(1000) {

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

    private val itemNoneInstalled = TextItem(R.string.no_modules_found)
    private val itemNoneUpdatable = TextItem(R.string.module_update_none)

    private val itemsInstalledHelpers = ObservableArrayList<TextItem>().also {
        it.add(itemNoneInstalled)
    }
    private val itemsUpdatableHelpers = ObservableArrayList<TextItem>().also {
        it.add(itemNoneUpdatable)
    }

    private val itemsCoreOnly = ObservableArrayList<SafeModeNotice>()
    private val itemsInstalled = diffListOf<ModuleItem>()
    private val itemsUpdatable = diffListOf<RepoItem.Update>()
    private val itemsRemote = diffListOf<RepoItem.Remote>()

    val adapter = adapterOf<ComparableRvItem<*>>()
    val items = MergeObservableList<ComparableRvItem<*>>()
        .insertList(itemsCoreOnly)
        .insertItem(sectionActive)
        .insertList(itemsInstalledHelpers)
        .insertList(itemsInstalled)
        .insertItem(InstallModule)
        .insertItem(sectionUpdate)
        .insertList(itemsUpdatableHelpers)
        .insertList(itemsUpdatable)
        .insertItem(sectionRemote)
        .insertList(itemsRemote)!!
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
            R.string.installed,
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

    private var remoteJob: Disposable? = null
    private var refetch = false
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

        itemsInstalled.addOnListChangedCallback(
            onItemRangeInserted = { sender, _, count ->
                if (count > 0 || sender.size > 0) {
                    itemsInstalledHelpers.clear()
                }
            }
        )
        itemsUpdatable.addOnListChangedCallback(
            onItemRangeInserted = { sender, _, count ->
                if (count > 0 || sender.size > 0) {
                    itemsUpdatableHelpers.clear()
                }
            }
        )
    }

    // ---

    override fun refresh(): Disposable {
        updateCoreOnlyWarning()
        if (itemsRemote.isEmpty())
            loadRemote()
        return loadInstalled().subscribeK()
    }

    private fun loadInstalled() = Single.fromCallable { Module.loadModules() }
        .map { it.map { ModuleItem(it) } }
        .map { it.loadDetail() }
        .map { it to itemsInstalled.calculateDiff(it) }
        .applyViewModel(this)
        .observeOn(AndroidSchedulers.mainThread())
        .map {
            itemsInstalled.update(it.first, it.second)
            it.first
        }
        .observeOn(Schedulers.io())
        .map { loadUpdates(it) }
        .map { it to itemsUpdatable.calculateDiff(it) }
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess { itemsUpdatable.update(it.first, it.second) }
        .ignoreElement()!!

    @Synchronized
    fun loadRemote() {
        // check for existing jobs
        if (remoteJob?.isDisposed?.not() == true) {
            return
        }
        if (itemsRemote.isEmpty()) {
            EndlessRecyclerScrollListener.ResetState().publish()
        }

        fun loadRemoteDB(offset: Int) = Single
            .fromCallable { dao.getRepos(offset) }
            .map { it.map { RepoItem.Remote(it) } }

        remoteJob = if (itemsRemote.isEmpty()) {
            repoUpdater(refetch).andThen(loadRemoteDB(0))
        } else {
            loadRemoteDB(itemsRemote.size)
        }.subscribeK(onError = Timber::e) {
            itemsRemote.addAll(it)
        }

        refetch = false
    }

    fun forceRefresh() {
        itemsRemote.clear()
        itemsSearch.clear()
        refetch = true
        refresh()
        submitQuery()
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

    @WorkerThread
    private fun List<ModuleItem>.loadDetail() = onEach { module ->
        Single.fromCallable { dao.getRepoById(module.item.id)!! }
            .subscribeK(onError = {}) { module.repo = it }
            .add()
    }

    private fun update(repo: Repo, progress: Int) =
        Single.fromCallable { itemsRemote + itemsSearch }
            .map { it.first { it.item.id == repo.id } }
            .subscribeK { it.progress.value = progress }
            .add()

    private fun updateCoreOnlyWarning() {
        if (Config.coreOnly) {
            if (itemsCoreOnly.isNotEmpty()) return
            itemsCoreOnly.add(SafeModeNotice)
        } else {
            itemsCoreOnly.clear()
        }
    }

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
                    itemsRemote.removeAll(it)
                    remoteJob?.dispose()
                    loadRemote()
                }.add()
        }
        else -> Unit
    }

    fun downloadPressed(item: RepoItem) = ModuleInstallDialog(item.item).publish()
    fun installPressed() = InstallExternalModuleEvent().publish()
    fun infoPressed(item: RepoItem) = OpenChangelogEvent(item.item).publish()
    fun infoPressed(item: ModuleItem) {
        OpenChangelogEvent(item.repo ?: return).publish()
    }

}
