package com.topjohnwu.magisk.ui.module

import android.content.res.Resources
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.ModuleRepository
import com.topjohnwu.magisk.model.entity.recycler.ModuleRvItem
import com.topjohnwu.magisk.model.entity.recycler.RepoRvItem
import com.topjohnwu.magisk.model.entity.recycler.SectionRvItem
import com.topjohnwu.magisk.model.events.InstallModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.model.events.OpenFilePickerEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.toSingle
import com.topjohnwu.magisk.utils.update
import com.topjohnwu.magisk.utils.zip
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposable
import io.reactivex.schedulers.Schedulers
import me.tatarka.bindingcollectionadapter2.OnItemBind

class ModuleViewModel(
    private val resources: Resources,
    private val moduleRepo: ModuleRepository
) : MagiskViewModel() {

    val query = KObservableField("")

    private val allItems = mutableListOf<ComparableRvItem<*>>()

    val itemsInstalled = DiffObservableList(ComparableRvItem.callback)
    val itemsRemote = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = OnItemBind<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@ModuleViewModel)
    }

    private var queryDisposable: Disposable? = null

    init {
        query.addOnPropertyChangedCallback {
            queryDisposable?.dispose()
            queryDisposable = query()
        }
        refresh(false)
    }

    fun fabPressed() = OpenFilePickerEvent().publish()
    fun repoPressed(item: RepoRvItem) = OpenChangelogEvent(item.item).publish()
    fun downloadPressed(item: RepoRvItem) = InstallModuleEvent(item.item).publish()

    fun refresh(forceReload: Boolean) {
        val updateInstalled = moduleRepo.fetchInstalledModules()
            .flattenAsFlowable { it }
            .map { ModuleRvItem(it) }
            .toList()
            .map { it to itemsInstalled.calculateDiff(it) }

        val updateRemote = moduleRepo.fetchModules(forceReload)

        zip(updateInstalled, updateRemote) { installed, remote -> installed to remote }
            .observeOn(AndroidSchedulers.mainThread())
            .map {
                itemsInstalled.update(it.first.first, it.first.second)
                it.second
            }
            .observeOn(Schedulers.computation())
            .flattenAsFlowable { it }
            .map { RepoRvItem(it) }
            .toList()
            .doOnSuccess { allItems.update(it) }
            .flatMap { queryRaw() }
            .applyViewModel(this)
            .subscribeK { itemsRemote.update(it.first, it.second) }
    }

    private fun query() = queryRaw()
        .subscribeK { itemsRemote.update(it.first, it.second) }

    private fun queryRaw(query: String = this.query.value) = allItems.toSingle()
        .map { it.filterIsInstance<RepoRvItem>() }
        .flattenAsFlowable { it }
        .filter {
            it.item.name.contains(query, ignoreCase = true) ||
                    it.item.author.contains(query, ignoreCase = true) ||
                    it.item.description.contains(query, ignoreCase = true)
        }
        .toList()
        .map { if (query.isEmpty()) it.divide() else it }
        .map { it to itemsRemote.calculateDiff(it) }

    private fun List<RepoRvItem>.divide(): List<ComparableRvItem<*>> {
        val installed = itemsInstalled.filterIsInstance<ModuleRvItem>()

        fun <T : ComparableRvItem<*>> List<T>.withTitle(text: Int) =
            if (isEmpty()) this else listOf(SectionRvItem(resources.getString(text))) + this

        val groupedItems = groupBy { repo ->
            installed.firstOrNull { it.item.id == repo.item.id }?.let {
                if (it.item.versionCode < repo.item.versionCode) MODULE_UPDATABLE
                else MODULE_INSTALLED
            } ?: MODULE_REMOTE
        }

        return groupedItems.getOrElse(MODULE_UPDATABLE) { listOf() }.withTitle(R.string.update_available) +
                groupedItems.getOrElse(MODULE_INSTALLED) { listOf() }.withTitle(R.string.installed) +
                groupedItems.getOrElse(MODULE_REMOTE) { listOf() }.withTitle(R.string.not_installed)
    }

    companion object {
        protected const val MODULE_INSTALLED = 0
        protected const val MODULE_REMOTE = 1
        protected const val MODULE_UPDATABLE = 2
    }

}