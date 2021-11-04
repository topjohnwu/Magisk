package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.os.Process
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.Queryable
import com.topjohnwu.magisk.databinding.filterableListOf
import com.topjohnwu.magisk.databinding.itemBindingOf
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.stream.Collectors

class DenyListViewModel : BaseViewModel(), Queryable {

    override val queryDelay = 0L

    var isShowSystem = false
        set(value) {
            field = value
            submitQuery()
        }

    var isShowOS = false
        set(value) {
            field = value
            submitQuery()
        }

    var query = ""
        set(value) {
            field = value
            submitQuery()
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
            val apps = pm.getInstalledApplications(MATCH_UNINSTALLED_PACKAGES).parallelStream()
                .filter { AppContext.packageName != it.packageName }
                .map { AppProcessInfo(it, pm, denyList) }
                .filter { it.processes.isNotEmpty() }
                .map { DenyListRvItem(it) }
                .sorted()
                .collect(Collectors.toList())
            apps to items.calculateDiff(apps)
        }
        items.update(apps, diff)
        submitQuery()
    }

    override fun query() {
        fun isApp(uid: Int) = run {
            val appId: Int = uid % 100000
            appId >= Process.FIRST_APPLICATION_UID && appId <= Process.LAST_APPLICATION_UID
        }

        fun isSystemApp(flag: Int) = flag and ApplicationInfo.FLAG_SYSTEM != 0

        items.filter {
            fun filterSystem() = isShowSystem || !isSystemApp(it.info.flags)

            fun filterOS() = (isShowSystem && isShowOS) || isApp(it.info.uid)

            fun filterQuery(): Boolean {
                fun inName() = it.info.label.contains(query, true)
                fun inPackage() = it.info.packageName.contains(query, true)
                fun inProcesses() = it.processes.any { p -> p.process.name.contains(query, true) }
                return inName() || inPackage() || inProcesses()
            }

            filterSystem() && filterOS() && filterQuery()
        }
        state = State.LOADED
    }
}
