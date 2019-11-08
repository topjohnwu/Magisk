package com.topjohnwu.magisk.redesign.module

import androidx.annotation.UiThread
import androidx.annotation.WorkerThread
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoByNameDao
import com.topjohnwu.magisk.data.database.RepoByUpdatedDao
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.module.Module
import com.topjohnwu.magisk.model.entity.module.Repo
import com.topjohnwu.magisk.model.entity.recycler.ModuleItem
import com.topjohnwu.magisk.model.entity.recycler.RepoItem
import com.topjohnwu.magisk.model.entity.recycler.SectionTitle
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.tasks.RepoUpdater
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.Completable
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposable
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter

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
        private val sectionActive = SectionTitle(R.string.module_section_active)
        private val sectionPending =
            SectionTitle(R.string.module_section_pending, R.string.reboot, R.drawable.ic_restart)
    }

    // ---

    private val itemsPending
        @WorkerThread get() = items.asSequence()
            .filterIsInstance<ModuleItem>()
            .filter { it.isModified }
            .toList()
    private val itemsActive
        @WorkerThread get() = items.asSequence()
            .filterIsInstance<ModuleItem>()
            .filter { !it.isModified }
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

    override fun refresh() = Single.fromCallable { Module.loadModules() }
        .map { it.map { ModuleItem(it) } }
        .map { it.order() }
        .map {
            val pending = it.getValue(ModuleState.Modified)
            val active = it.getValue(ModuleState.Normal)
            build(pending = pending, active = active)
        }
        .map { it to items.calculateDiff(it) }
        .subscribeK {
            items.update(it.first, it.second)
            if (!items.contains(sectionRemote)) {
                loadRemote()
            }
        }

    @Synchronized
    fun loadRemote() {
        // check for existing jobs
        val size = itemsRemote.size
        if (remoteJob?.isDisposed?.not() == true || size % 10 != 0) {
            return
        }
        remoteJob = loadRepos(offset = size)
            .map { it.map { RepoItem(it) } }
            .applyViewModel(this)
            .subscribeK {
                if (!items.contains(sectionRemote)) {
                    items.add(sectionRemote)
                }
                items.addAll(it)
            }
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
        .groupBy {
            when {
                it.isModified -> ModuleState.Modified
                else -> ModuleState.Normal
            }
        }
        .ensureAllStates()

    private fun Map<ModuleState, List<ModuleItem>>.ensureAllStates(): Map<ModuleState, List<ModuleItem>> {
        val me = this as? MutableMap<ModuleState, List<ModuleItem>> ?: this.toMutableMap()
        ModuleState.values().forEach {
            if (me.none { rit -> it == rit.key }) {
                me[it] = listOf()
            }
        }
        return me
    }

    // ---

    @UiThread
    fun moveToState(item: ModuleItem) {
        items.removeAll { it.genericItemSameAs(item) }

        val isPending = item.isModified

        Single.fromCallable { if (isPending) itemsPending else itemsActive }
            .map { (listOf(item) + it).toMutableList() }
            .map { it.apply { sortWith(compareBy { it.item.name.toLowerCase(currentLocale) }) } }
            .map {
                if (isPending) build(pending = it)
                else build(active = it)
            }
            .map { it to items.calculateDiff(it) }
            .observeOn(AndroidSchedulers.mainThread())
            .doOnSuccess { items.update(it.first, it.second) }
            .ignoreElement()
            .andThen(cleanup())
            .subscribeK()
    }

    // ---

    private fun cleanup() = Completable
        .concat(listOf(cleanPending(), cleanActive(), cleanRemote()))

    private fun cleanPending() = Single.fromCallable { itemsPending }
        .filter { it.isEmpty() }
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess { items.remove(sectionPending) }
        .ignoreElement()

    private fun cleanActive() = Single.fromCallable { itemsActive }
        .filter { it.isEmpty() }
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess { items.remove(sectionActive) }
        .ignoreElement()

    private fun cleanRemote() = Single.fromCallable { itemsRemote }
        .filter { it.isEmpty() }
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess { items.remove(sectionRemote) }
        .ignoreElement()

    // ---

    private enum class ModuleState {
        Modified, Normal
    }

    // ---

    /** Callable only from worker thread because of expensive list filtering */
    @WorkerThread
    private fun build(
        pending: List<ModuleItem> = itemsPending,
        active: List<ModuleItem> = itemsActive,
        remote: List<RepoItem> = itemsRemote
    ) = pending.prependIfNotEmpty { sectionPending } +
            active.prependIfNotEmpty { sectionActive } +
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