package com.topjohnwu.magisk.redesign.module

import androidx.annotation.WorkerThread
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
import com.topjohnwu.magisk.model.entity.recycler.LoadingItem
import com.topjohnwu.magisk.model.entity.recycler.ModuleItem
import com.topjohnwu.magisk.model.entity.recycler.RepoItem
import com.topjohnwu.magisk.model.entity.recycler.SectionTitle
import com.topjohnwu.magisk.model.events.dialog.ModuleInstallDialog
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.tasks.RepoUpdater
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.Single
import io.reactivex.disposables.Disposable
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter
import timber.log.Timber
import kotlin.math.roundToInt

class ModuleViewModel(
    private val repoName: RepoByNameDao,
    private val repoUpdated: RepoByUpdatedDao,
    private val repoUpdater: RepoUpdater
) : CompatViewModel() {

    val adapter = adapterOf<ComparableRvItem<*>>()
    val items = diffListOf<ComparableRvItem<*>>()
    val itemBinding = itemBindingOf<ComparableRvItem<*>> {
        it.bindExtra(BR.viewModel, this)
    }

    companion object {
        private val sectionRemote = SectionTitle(R.string.module_section_remote)
        private val sectionActive = SectionTitle(
            R.string.module_section_installed,
            R.string.reboot,
            R.drawable.ic_restart
        ).also { it.hasButton.value = false }
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
        .subscribeK {
            items.update(it.first, it.second)
            if (!items.contains(sectionRemote)) {
                loadRemote()
            }
            moveToState()
        }

    @Synchronized
    fun loadRemote() {
        // check for existing jobs
        if (remoteJob?.isDisposed?.not() == true) {
            return
        }
        remoteJob = Single.fromCallable { itemsRemote.size }
            .flatMap { loadRepos(offset = it) }
            .map { it.map { RepoItem(it) } }
            .subscribeK(onError = {
                Timber.e(it)
                items.remove(LoadingItem)
            }) {
                items.remove(LoadingItem)
                if (!items.contains(sectionRemote)) {
                    items.add(sectionRemote)
                }
                items.addAll(it)
            }
        // do on subscribe doesn't perform the action on main thread, so this is perfectly fine
        items.add(LoadingItem)
    }

    private fun loadRepos(
        offset: Int = 0,
        downloadRepos: Boolean = offset == 0
    ): Single<List<Repo>> = Single.fromCallable { dao.getRepos(offset) }.flatMap {
        when {
            // in case we find result empty and offset is initial we need to refresh the repos.
            downloadRepos && it.isEmpty() && offset == 0 -> downloadRepos()
                .andThen(loadRepos(downloadRepos = false))
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

    private fun update(repo: Repo, progress: Int) = Single.fromCallable { itemsRemote }
        .map { it.first { it.item.id == repo.id } }
        .subscribeK { it.progress.value = progress }
        .add()

    // ---

    fun moveToState() = Single.fromCallable { itemsInstalled.any { it.isModified } }
        .subscribeK { sectionActive.hasButton.value = it }
        .add()

    fun download(item: RepoItem) = ModuleInstallDialog(item.item).publish()

    fun sectionPressed(item: SectionTitle) = when (item) {
        sectionActive -> reboot()
        else -> Unit
    }

    // ---

    /** Callable only from worker thread because of expensive list filtering */
    @WorkerThread
    private fun build(
        active: List<ModuleItem> = itemsInstalled,
        remote: List<RepoItem> = itemsRemote
    ) = active.prependIfNotEmpty { sectionActive } +
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