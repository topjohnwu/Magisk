package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.databinding.filterableListOf
import com.topjohnwu.magisk.databinding.itemBindingOf
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.concurrentMap
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.*
import java.util.*

class DenyListViewModel : BaseViewModel() {

    var isShowSystem = false
        set(value) {
            field = value
            query()
        }

    var isShowOS = false
        set(value) {
            field = value
            query()
        }

    var query = ""
        set(value) {
            field = value
            query()
        }

    val items = filterableListOf<DenyListRvItem>()
    val itemBinding = itemBindingOf<DenyListRvItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<ProcessRvItem> {
        it.bindExtra(BR.viewModel, this)
    }

    @SuppressLint("InlinedApi")
    override fun refresh() = viewModelScope.launch {
        if (!Utils.showSuperUser()) {
            state = State.LOADING_FAILED
            return@launch
        }
        state = State.LOADING
        val (apps, diff) = withContext(Dispatchers.Default) {
            val pm = AppContext.packageManager
            val denyList = Shell.su("magisk --denylist ls").exec().out
                .map { CmdlineListItem(it) }
            val apps = pm.getInstalledApplications(MATCH_UNINSTALLED_PACKAGES).run {
                asFlow()
                    .filter { AppContext.packageName != it.packageName }
                    .concurrentMap { AppProcessInfo(it, pm, denyList) }
                    .filter { it.processes.isNotEmpty() }
                    .concurrentMap { DenyListRvItem(it) }
                    .toCollection(ArrayList(size))
            }
            apps.sort()
            apps to items.calculateDiff(apps)
        }
        items.update(apps, diff)
        query()
    }

    fun query() {
        items.filter {
            fun filterSystem() = isShowSystem || !it.info.isSystemApp()

            fun filterOS() = (isShowSystem && isShowOS) || it.info.isApp()

            fun filterQuery(): Boolean {
                fun inName() = it.info.label.contains(query, true)
                fun inPackage() = it.info.packageName.contains(query, true)
                fun inProcesses() = it.processes.any { p -> p.process.name.contains(query, true) }
                return inName() || inPackage() || inProcesses()
            }

            (it.isChecked || (filterSystem() && filterOS())) && filterQuery()
        }
        state = State.LOADED
    }
}
