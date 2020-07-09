package com.topjohnwu.magisk.ui.module

import androidx.databinding.Bindable
import androidx.databinding.ObservableArrayList
import androidx.databinding.ObservableField
import androidx.lifecycle.viewModelScope
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
import com.topjohnwu.magisk.extensions.value
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.recycler.*
import com.topjohnwu.magisk.model.events.InstallExternalModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.dialog.ModuleInstallDialog
import com.topjohnwu.magisk.ui.base.*
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import kotlinx.coroutines.*
import me.tatarka.bindingcollectionadapter2.collections.MergeObservableList
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
) : BaseViewModel(), Queryable {

    override val queryDelay = 1000L
    private var queryJob: Job? = null
    private var remoteJob: Job? = null

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

    val searchLoading = ObservableField(false)
    val itemsSearch = diffListOf<RepoItem>()
    val itemSearchBinding = itemBindingOf<RepoItem> {
        it.bindExtra(BR.viewModel, this)
    }

    private val itemNoneInstalled = TextItem(R.string.no_modules_found)
    private val itemNoneUpdatable = TextItem(R.string.module_update_none)

    private val itemsInstalledHelpers = ObservableArrayList<TextItem>()
    private val itemsUpdatableHelpers = ObservableArrayList<TextItem>()

    private val itemsInstalled = diffListOf<ModuleItem>()
    private val itemsUpdatable = diffListOf<RepoItem.Update>()
    private val itemsRemote = diffListOf<RepoItem.Remote>()

    var isRemoteLoading = false
        @Bindable get
        private set(value) {
            field = value
            notifyPropertyChanged(BR.remoteLoading)
        }

    val adapter = adapterOf<ComparableRvItem<*>>()
    val items = MergeObservableList<ComparableRvItem<*>>()
        .insertItem(InstallModule)
        .insertItem(sectionUpdate)
        .insertList(itemsUpdatableHelpers)
        .insertList(itemsUpdatable)
        .insertItem(sectionActive)
        .insertList(itemsInstalledHelpers)
        .insertList(itemsInstalled)
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
            R.string.module_installed,
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
            onItemRangeInserted = { _, _, _ -> itemsInstalledHelpers.clear() },
            onItemRangeRemoved = { _, _, _ -> addInstalledEmptyMessage() }
        )
        itemsUpdatable.addOnListChangedCallback(
            onItemRangeInserted = { _, _, _ -> itemsUpdatableHelpers.clear() },
            onItemRangeRemoved = { _, _, _ -> addUpdatableEmptyMessage() }
        )
    }

    // ---

    override fun refresh(): Job {
        if (itemsRemote.isEmpty())
            loadRemote()
        return loadInstalled()
    }

    private suspend fun loadUpdates(installed: List<ModuleItem>) = withContext(Dispatchers.IO) {
        installed
            .mapNotNull { dao.getUpdatableRepoById(it.item.id, it.item.versionCode) }
            .map { RepoItem.Update(it) }
    }

    private suspend fun List<ModuleItem>.loadDetails() = withContext(Dispatchers.IO) {
        onEach {
            launch {
                it.repo = dao.getRepoById(it.item.id)
            }
        }
    }

    private fun loadInstalled() = viewModelScope.launch {
        state = State.LOADING
        val installed = Module.installed().map { ModuleItem(it) }
        val detailLoad = async { installed.loadDetails() }
        val updates = loadUpdates(installed)
        val diff = withContext(Dispatchers.Default) {
            val i = async { itemsInstalled.calculateDiff(installed) }
            val u = async { itemsUpdatable.calculateDiff(updates) }
            awaitAll(i, u)
        }
        detailLoad.await()
        itemsInstalled.update(installed, diff[0])
        itemsUpdatable.update(updates, diff[1])
        addInstalledEmptyMessage()
        addUpdatableEmptyMessage()
        updateActiveState()
        state = State.LOADED
    }

    fun loadRemote() {
        // check for existing jobs
        if (remoteJob?.isActive == true)
            return

        if (itemsRemote.isEmpty())
            EndlessRecyclerScrollListener.ResetState().publish()

        remoteJob = viewModelScope.launch {
            suspend fun loadRemoteDB(offset: Int) = withContext(Dispatchers.IO) {
                dao.getRepos(offset).map { RepoItem.Remote(it) }
            }

            isRemoteLoading = true
            val repos = if (itemsRemote.isEmpty()) {
                repoUpdater(refetch)
                loadRemoteDB(0)
            } else {
                loadRemoteDB(itemsRemote.size)
            }
            isRemoteLoading = false
            refetch = false
            queryHandler.post { itemsRemote.addAll(repos) }
        }
    }

    fun forceRefresh() {
        itemsRemote.clear()
        itemsSearch.clear()
        refetch = true
        refresh()
        submitQuery()
    }

    // ---

    private suspend fun queryInternal(query: String, offset: Int): List<RepoItem> {
        return if (query.isBlank()) {
            itemsSearch.clear()
            listOf()
        } else {
            withContext(Dispatchers.IO) {
                dao.searchRepos(query, offset).map { RepoItem.Remote(it) }
            }
        }
    }

    override fun query() {
        EndlessRecyclerScrollListener.ResetState().publish()
        queryJob = viewModelScope.launch {
            val searched = queryInternal(query, 0)
            val diff = withContext(Dispatchers.Default) {
                itemsSearch.calculateDiff(searched)
            }
            searchLoading.value = false
            itemsSearch.update(searched, diff)
        }
    }

    fun loadMoreQuery() {
        if (queryJob?.isActive == true) return
        queryJob = viewModelScope.launch {
            val searched = queryInternal(query, itemsSearch.size)
            queryHandler.post { itemsSearch.addAll(searched) }
        }
    }

    // ---

    private fun update(repo: Repo, progress: Int) = viewModelScope.launch {
        val items = withContext(Dispatchers.Default) {
            val predicate = { it: RepoItem -> it.item.id == repo.id }
            itemsUpdatable.filter(predicate) +
                itemsRemote.filter(predicate) +
                itemsSearch.filter(predicate)
        }
        items.forEach { it.progress.value = progress }
    }

    // ---

    private fun addInstalledEmptyMessage() {
        if (itemsInstalled.isEmpty() && itemsInstalledHelpers.isEmpty()) {
            itemsInstalledHelpers.add(itemNoneInstalled)
        }
    }

    private fun addUpdatableEmptyMessage() {
        if (itemsUpdatable.isEmpty() && itemsUpdatableHelpers.isEmpty()) {
            itemsUpdatableHelpers.add(itemNoneUpdatable)
        }
    }

    // ---

    fun updateActiveState() = viewModelScope.launch {
        sectionActive.hasButton = withContext(Dispatchers.Default) {
            itemsInstalled.any { it.isModified }
        }
    }

    fun sectionPressed(item: SectionTitle) = when (item) {
        sectionActive -> reboot() // TODO add reboot picker, regular reboot is not always preferred
        sectionRemote -> {
            Config.repoOrder = when (Config.repoOrder) {
                Config.Value.ORDER_NAME -> Config.Value.ORDER_DATE
                Config.Value.ORDER_DATE -> Config.Value.ORDER_NAME
                else -> Config.Value.ORDER_NAME
            }
            updateOrderIcon()
            queryHandler.post {
                itemsRemote.clear()
                loadRemote()
            }
            Unit
        }
        else -> Unit
    }

    fun downloadPressed(item: RepoItem) = withExternalRW {
        if (it)
            ModuleInstallDialog(item.item).publish()
        else
            permissionDenied()
    }

    fun installPressed() = withExternalRW {
        if (it)
            InstallExternalModuleEvent().publish()
        else
            permissionDenied()
    }

    fun infoPressed(item: RepoItem) = OpenChangelogEvent(item.item).publish()
    fun infoPressed(item: ModuleItem) {
        OpenChangelogEvent(item.repo ?: return).publish()
    }

    private fun permissionDenied() {
        SnackbarEvent(R.string.module_permission_declined).publish()
    }

}
