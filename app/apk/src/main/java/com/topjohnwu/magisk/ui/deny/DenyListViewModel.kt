package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
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
import kotlinx.coroutines.withContext

enum class SortBy { NAME, PACKAGE_NAME, INSTALL_TIME, UPDATE_TIME }

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
        val comparator = compareBy<DenyAppState> { it.itemsChecked == 0 }
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
                    .toCollection(ArrayList(size))
            }
            apps.sortWith(compareBy(
                { it.processes.count { p -> p.isEnabled } == 0 },
                { it.info }
            ))
            apps
        }
        _allApps.value = apps
        _loading.value = false
    }
}

class DenyAppState(val info: AppProcessInfo) : Comparable<DenyAppState> {
    val processes = info.processes.map { DenyProcessState(it) }
    var isExpanded by mutableStateOf(false)

    val itemsChecked: Int get() = processes.count { it.isEnabled }
    val isChecked: Boolean get() = itemsChecked > 0
    val checkedPercent: Float get() = if (processes.isEmpty()) 0f else itemsChecked.toFloat() / processes.size

    fun toggleAll() {
        if (isChecked) {
            Shell.cmd("magisk --denylist rm ${info.packageName}").submit()
            processes.filter { it.isEnabled }.forEach { proc ->
                if (proc.process.isIsolated) {
                    proc.toggle()
                } else {
                    proc.isEnabled = false
                }
            }
        } else {
            processes.filterNot { it.isEnabled }.forEach { it.toggle() }
        }
    }

    override fun compareTo(other: DenyAppState) = comparator.compare(this, other)

    companion object {
        private val comparator = compareBy<DenyAppState>(
            { it.itemsChecked == 0 },
            { it.info }
        )
    }
}

class DenyProcessState(val process: ProcessInfo) {
    var isEnabled by mutableStateOf(process.isEnabled)

    val displayName: String =
        if (process.isIsolated) "(isolated) ${process.name}*" else process.name

    fun toggle() {
        isEnabled = !isEnabled
        val arg = if (isEnabled) "add" else "rm"
        val (name, pkg) = process
        Shell.cmd("magisk --denylist $arg $pkg \'$name\'").submit()
    }
}
