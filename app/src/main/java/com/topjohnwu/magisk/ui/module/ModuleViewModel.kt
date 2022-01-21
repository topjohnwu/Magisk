package com.topjohnwu.magisk.ui.module

import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.databinding.RvItem
import com.topjohnwu.magisk.databinding.adapterOf
import com.topjohnwu.magisk.databinding.diffListOf
import com.topjohnwu.magisk.databinding.itemBindingOf
import com.topjohnwu.magisk.events.SelectModuleEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.events.dialog.ModuleInstallDialog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import me.tatarka.bindingcollectionadapter2.collections.MergeObservableList

class ModuleViewModel : BaseViewModel() {

    val bottomBarBarrierIds = intArrayOf(R.id.module_update, R.id.module_remove)

    private val itemsInstalled = diffListOf<LocalModuleRvItem>()

    val adapter = adapterOf<RvItem>()
    val items = MergeObservableList<RvItem>()
    val itemBinding = itemBindingOf<RvItem> {
        it.bindExtra(BR.viewModel, this)
    }

    init {
        if (Info.env.isActive) {
            items.insertItem(InstallModule)
                .insertList(itemsInstalled)
        }
    }

    override fun refresh(): Job {
        return viewModelScope.launch {
            state = State.LOADING
            loadInstalled()
            state = State.LOADED
            loadUpdateInfo()
        }
    }

    private suspend fun loadInstalled() {
        val installed = LocalModule.installed().map { LocalModuleRvItem(it) }
        val diff = withContext(Dispatchers.Default) {
            itemsInstalled.calculateDiff(installed)
        }
        itemsInstalled.update(installed, diff)
    }

    private suspend fun loadUpdateInfo() {
        withContext(Dispatchers.IO) {
            itemsInstalled.forEach {
                if (it.item.fetch())
                    it.fetchedUpdateInfo()
            }
        }
    }

    fun downloadPressed(item: OnlineModule?) =
        if (item != null && isConnected.get()) {
            withExternalRW { ModuleInstallDialog(item).publish() }
        } else {
            SnackbarEvent(R.string.no_connection).publish()
        }

    fun installPressed() = withExternalRW { SelectModuleEvent().publish() }

}
