package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.Queryable
import com.topjohnwu.magisk.arch.filterableListOf
import com.topjohnwu.magisk.arch.itemBindingOf
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ktx.packageInfo
import com.topjohnwu.magisk.ktx.packageName
import com.topjohnwu.magisk.ktx.processes
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class HideViewModel : BaseViewModel(), Queryable {

    override val queryDelay = 1000L

    @get:Bindable
    var isShowSystem = Config.showSystemApp
        set(value) = set(value, field, { field = it }, BR.showSystem){
            Config.showSystemApp = it
            submitQuery()
        }

    @get:Bindable
    var query = ""
        set(value) = set(value, field, { field = it }, BR.query){
            submitQuery()
        }

    val items = filterableListOf<HideItem>()
    val itemBinding = itemBindingOf<HideItem> {
        it.bindExtra(BR.viewModel, this)
    }
    val itemInternalBinding = itemBindingOf<HideProcessItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = viewModelScope.launch {
        state = State.LOADING
        val (apps, diff) = withContext(Dispatchers.Default) {
            val pm = get<PackageManager>()
            val hides = Shell.su("magiskhide --ls").exec().out.map { HideTarget(it) }
            val apps = pm.getInstalledApplications(0)
                .asSequence()
                .filter { it.enabled && !blacklist.contains(it.packageName) }
                .map { HideAppInfo(it, pm) }
                .map { createTarget(it, hides) }
                .filter { it.processes.isNotEmpty() }
                .map { HideItem(it) }
                .toList()
                .sorted()
            apps to items.calculateDiff(apps)
        }
        items.update(apps, diff)
        submitQuery()
    }

    // ---

    private fun createTarget(info: HideAppInfo, hideList: List<HideTarget>): HideAppTarget {
        val pkg = info.packageName
        val hidden = hideList.filter { it.packageName == pkg }
        val processNames = info.packageInfo.processes.distinct()
        val processes = processNames.map { name ->
            HideProcessInfo(name, pkg, hidden.any { name == it.process })
        }
        return HideAppTarget(info, processes)
    }

    // ---

    override fun query() {
        items.filter {
            fun showHidden() = it.itemsChecked != 0

            fun filterSystem() =
                isShowSystem || it.info.flags and ApplicationInfo.FLAG_SYSTEM == 0

            fun filterQuery(): Boolean {
                fun inName() = it.info.label.contains(query, true)
                fun inPackage() = it.info.packageName.contains(query, true)
                fun inProcesses() = it.processes.any { p -> p.process.name.contains(query, true) }
                return inName() || inPackage() || inProcesses()
            }

            showHidden() || (filterSystem() && filterQuery())
        }
        state = State.LOADED
    }

    // ---

    fun resetQuery() {
        query = ""
    }

    companion object {
        private val blacklist by lazy { listOf(
            packageName,
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
        ) }
    }
}

