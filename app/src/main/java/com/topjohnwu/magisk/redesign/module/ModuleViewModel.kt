package com.topjohnwu.magisk.redesign.module

import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.module.Module
import com.topjohnwu.magisk.model.entity.recycler.ModuleItem
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import io.reactivex.Single

class ModuleViewModel : CompatViewModel() {

    val items = diffListOf<ModuleItem>()
    val itemsPending = diffListOf<ModuleItem>()
    val itemBinding = itemBindingOf<ModuleItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = Single.fromCallable { Module.loadModules() }
        .map { it.map { ModuleItem(it) } }
        .map { it to items.calculateDiff(it) }
        .subscribeK {
            items.update(it.first, it.second)
            items.forEach { moveToState(it) }
        }

    fun moveToState(item: ModuleItem) {
        val isActive = items.indexOfFirst { it.itemSameAs(item) } != -1
        val isPending = itemsPending.indexOfFirst { it.itemSameAs(item) } != -1

        when {
            isActive && isPending -> if (item.isModified) {
                items.remove(item)
            } else {
                itemsPending.remove(item)
            }
            isActive && item.isModified -> {
                items.remove(item)
                itemsPending.add(item)
            }
            isPending && !item.isModified -> {
                itemsPending.remove(item)
                items.add(item)
            }
        }
    }

}