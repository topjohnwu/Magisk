package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.ktx.concurrentMap
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.filter
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.flow.toCollection
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.withContext

enum class SortBy { NAME, PACKAGE_NAME, INSTALL_TIME, UPDATE_TIME }

data class DenyProcessState(
    val process: ProcessInfo,
    val isEnabled: Boolean = process.isEnabled,
) {
    val displayName: String =
        if (process.isIsolated) "(isolated) ${process.name}*" else process.name
}

data class DenyAppState(
    val info: AppProcessInfo,
    val processes: List<DenyProcessState> = info.processes.map { DenyProcessState(it) },
    val isExpanded: Boolean = false,
    val initiallyChecked: Boolean = info.processes.any { it.isEnabled },
) : Comparable<DenyAppState> {

    val itemsChecked: Int get() = processes.count { it.isEnabled }
    val isChecked: Boolean get() = itemsChecked > 0
    val checkedPercent: Float get() = if (processes.isEmpty()) 0f else itemsChecked.toFloat() / processes.size

    override fun compareTo(other: DenyAppState) = comparator.compare(this, other)

    companion object {
        private val comparator = compareBy<DenyAppState>(
            { !it.initiallyChecked },
            { it.info }
        )
    }
}

class DenyListViewModel : AsyncLoadViewModel() {

    private val _loading = MutableStateFlow(true)
    val loading: StateFlow<Boolean> = _loading.asStateFlow()

    private val _allApps = MutableStateFlow<List<DenyAppState>>(emptyList())

    private val _query = MutableStateFlow("")
    val query: StateFlow<String> = _query.asStateFlow()

    private val _showSystem = MutableStateFlow(false)
    val showSystem: StateFlow<Boolean> = _showSystem.asStateFlow()

    private val _showOS = MutableStateFlow(false)
    val showOS: StateFlow<Boolean> = _showOS.asStateFlow()

    private val _sortBy = MutableStateFlow(SortBy.NAME)
    val sortBy: StateFlow<SortBy> = _sortBy.asStateFlow()

    private val _sortReverse = MutableStateFlow(false)
    val sortReverse: StateFlow<Boolean> = _sortReverse.asStateFlow()

    val filteredApps: StateFlow<List<DenyAppState>> = combine(
        _allApps, _query, _showSystem, _showOS, _sortBy, _sortReverse
    ) { args ->
        @Suppress("UNCHECKED_CAST")
        val apps = args[0] as List<DenyAppState>
        val q = args[1] as String
        val showSys = args[2] as Boolean
        val showOS = args[3] as Boolean
        val sort = args[4] as SortBy
        val reverse = args[5] as Boolean

        val filtered = apps.filter { app ->
            val passFilter = app.isChecked ||
                app.initiallyChecked ||
                ((showSys || !app.info.isSystemApp()) &&
                ((showSys && showOS) || app.info.isApp()))
            val passQuery = q.isBlank() ||
                app.info.label.contains(q, true) ||
                app.info.packageName.contains(q, true) ||
                app.processes.any { it.process.name.contains(q, true) }
            passFilter && passQuery
        }

        val secondary: Comparator<DenyAppState> = when (sort) {
            SortBy.NAME -> compareBy(String.CASE_INSENSITIVE_ORDER) { it.info.label }
            SortBy.PACKAGE_NAME -> compareBy(String.CASE_INSENSITIVE_ORDER) { it.info.packageName }
            SortBy.INSTALL_TIME -> compareByDescending { it.info.firstInstallTime }
            SortBy.UPDATE_TIME -> compareByDescending { it.info.lastUpdateTime }
        }
        val comparator = compareBy<DenyAppState> { !it.initiallyChecked }
            .then(if (reverse) secondary.reversed() else secondary)
        filtered.sortedWith(comparator)
    }.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5000), emptyList())

    fun setQuery(q: String) { _query.value = q }
    fun setShowSystem(v: Boolean) {
        _showSystem.value = v
        if (!v) _showOS.value = false
    }
    fun setShowOS(v: Boolean) { _showOS.value = v }
    fun setSortBy(s: SortBy) { _sortBy.value = s }
    fun toggleSortReverse() { _sortReverse.value = !_sortReverse.value }

    fun toggleExpanded(app: DenyAppState) {
        _allApps.update { apps ->
            apps.map {
                if (it.info.packageName == app.info.packageName) it.copy(isExpanded = !it.isExpanded) else it
            }
        }
    }

    fun toggleAll(app: DenyAppState) {
        val willCheck = !app.isChecked
        if (!willCheck) {
            Shell.cmd("magisk --denylist rm ${app.info.packageName}").submit()
        }
        _allApps.update { apps ->
            apps.map { currentApp ->
                if (currentApp.info.packageName == app.info.packageName) {
                    val newProcs = currentApp.processes.map { proc ->
                        if (willCheck) {
                            if (!proc.isEnabled) {
                                val (name, pkg) = proc.process
                                Shell.cmd("magisk --denylist add $pkg '$name'").submit()
                            }
                            proc.copy(isEnabled = true)
                        } else {
                            if (proc.process.isIsolated && proc.isEnabled) {
                                val (name, pkg) = proc.process
                                Shell.cmd("magisk --denylist rm $pkg '$name'").submit()
                            }
                            proc.copy(isEnabled = false)
                        }
                    }
                    currentApp.copy(processes = newProcs)
                } else {
                    currentApp
                }
            }
        }
    }

    fun toggleProcess(app: DenyAppState, proc: DenyProcessState) {
        val newEnabled = !proc.isEnabled
        val arg = if (newEnabled) "add" else "rm"
        val (name, pkg) = proc.process
        Shell.cmd("magisk --denylist $arg $pkg '$name'").submit()

        _allApps.update { apps ->
            apps.map { currentApp ->
                if (currentApp.info.packageName == app.info.packageName) {
                    val newProcs = currentApp.processes.map { currentProc ->
                        if (currentProc.process.name == proc.process.name && currentProc.process.packageName == proc.process.packageName) {
                            currentProc.copy(isEnabled = newEnabled)
                        } else {
                            currentProc
                        }
                    }
                    currentApp.copy(processes = newProcs)
                } else {
                    currentApp
                }
            }
        }
    }

    @SuppressLint("InlinedApi")
    override suspend fun doLoadWork() {
        _loading.value = true
        val apps = withContext(Dispatchers.Default) {
            val pm = AppContext.packageManager
            val denyList = Shell.cmd("magisk --denylist ls").exec().out
                .map { CmdlineListItem(it) }
            val apps = pm.getInstalledApplications(MATCH_UNINSTALLED_PACKAGES).run {
                asFlow()
                    .filter { AppContext.packageName != it.packageName }
                    .concurrentMap { AppProcessInfo(it, pm, denyList) }
                    .filter { it.processes.isNotEmpty() }
                    .concurrentMap { DenyAppState(it) }
                    .toCollection(ArrayList(size + 1))
            }
            apps += DenyAppState(
                AppProcessInfo.webViewZygote(
                    pm,
                    denyList,
                    "WebView Zygote",
                )
            )
            apps.sortWith(compareBy(
                { !it.initiallyChecked },
                { it.info }
            ))
            apps
        }
        _allApps.value = apps
        _loading.value = false
    }
}
