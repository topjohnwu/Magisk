package com.topjohnwu.magisk.ui.module

import android.content.res.Resources
import android.database.Cursor
import androidx.annotation.StringRes
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.doOnSuccessUi
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
import io.reactivex.disposables.Disposable
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
        refresh()
    }

    fun fabPressed() = OpenFilePickerEvent().publish()
    fun repoPressed(item: RepoRvItem) = OpenChangelogEvent(item.item).publish()
    fun downloadPressed(item: RepoRvItem) = InstallModuleEvent(item.item).publish()

    fun refresh() {
        val updateInstalled = moduleRepo.fetchInstalledModules()
            .flattenAsFlowable { it }
            .map { ModuleRvItem(it) }
            .toList()
            .map { it to itemsInstalled.calculateDiff(it) }
            .doOnSuccessUi { itemsInstalled.update(it.first, it.second) }

        val updateRemote = moduleRepo.fetchModules()

        zip(updateInstalled, updateRemote) { _, remote -> remote }
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
        val installedModules = filter { installed.any { item -> it.item.id == item.item.id } }

        fun installedByID(id: String) = installed.firstOrNull { it.item.id == id }

        fun List<RepoRvItem>.filterObsolete() = filter {
            val module = installedByID(it.item.id) ?: return@filter false
            module.item.versionCode != it.item.versionCode
        }

        val resultObsolete = installedModules.filterObsolete()
        val resultInstalled = installedModules - resultObsolete
        val resultRemote = toList() - installedModules

        fun buildList(@StringRes text: Int, list: List<RepoRvItem>): List<ComparableRvItem<*>> {
            return if (list.isEmpty()) list
            else listOf(SectionRvItem(resources.getString(text))) + list
        }

        return buildList(R.string.update_available, resultObsolete) +
                buildList(R.string.installed, resultInstalled) +
                buildList(R.string.not_installed, resultRemote)
    }

    private fun <Result> Cursor.toList(transformer: (Cursor) -> Result): List<Result> {
        val out = mutableListOf<Result>()
        while (moveToNext()) out.add(transformer(this))
        return out
    }

}