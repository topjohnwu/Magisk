package com.topjohnwu.magisk.redesign.module

import androidx.annotation.WorkerThread
import androidx.recyclerview.widget.DiffUtil
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.module.Module
import com.topjohnwu.magisk.model.entity.recycler.ModuleItem
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.currentLocale
import io.reactivex.Single

class ModuleViewModel : CompatViewModel() {

    val items = diffListOf<ModuleItem>()
    val itemsPending = diffListOf<ModuleItem>()
    val itemBinding = itemBindingOf<ModuleItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = Single.fromCallable { Module.loadModules() }
        .map { it.map { ModuleItem(it) } }
        .map { it.order() }
        .subscribeK { it.forEach { it.update() } }

    @WorkerThread
    private fun List<ModuleItem>.order() = sortedBy { it.item.name.toLowerCase(currentLocale) }
        .groupBy {
            when {
                it.isModified -> ModuleState.Modified
                else -> ModuleState.Normal
            }
        }
        .map {
            val diff = when (it.key) {
                ModuleState.Modified -> itemsPending
                ModuleState.Normal -> items
            }.calculateDiff(it.value)
            ResultEnclosure(it.key, it.value, diff)
        }
        .ensureAllStates()

    private fun List<ResultEnclosure>.ensureAllStates(): List<ResultEnclosure> {
        val me = this as? MutableList<ResultEnclosure> ?: this.toMutableList()
        ModuleState.values().forEach {
            if (me.none { rit -> it == rit.state }) {
                me.add(ResultEnclosure(it, listOf(), null))
            }
        }
        return me
    }

    fun moveToState(item: ModuleItem) {
        items.removeAll { it.itemSameAs(item) }
        itemsPending.removeAll { it.itemSameAs(item) }

        if (item.isModified) {
            itemsPending
        } else {
            items
        }.apply {
            add(item)
            sortWith(compareBy { it.item.name.toLowerCase(currentLocale) })
        }
    }

    private enum class ModuleState {
        Modified, Normal
    }

    private data class ResultEnclosure(
        val state: ModuleState,
        val list: List<ModuleItem>,
        val diff: DiffUtil.DiffResult?
    )

    private fun ResultEnclosure.update() = when (state) {
        ModuleState.Modified -> itemsPending
        ModuleState.Normal -> items
    }.update(list, diff)

    private fun <T> DiffObservableList<T>.update(list: List<T>, diff: DiffUtil.DiffResult?) {
        diff ?: let {
            update(list)
            return
        }
        update(list, diff)
    }

}