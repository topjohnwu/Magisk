package com.topjohnwu.magisk.ui.superuser

import android.annotation.SuppressLint
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.graphics.drawable.Drawable
import androidx.core.os.ProcessCompat
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.R as CoreR
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.ktx.concurrentMap
import com.topjohnwu.magisk.core.ktx.getLabel
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.su.SuEvents
import kotlinx.coroutines.CancellationException
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
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.util.Locale

enum class SortBy { NAME, PACKAGE_NAME, INSTALL_TIME, UPDATE_TIME }

data class SuGrantAppInfo(
    val label: String,
    val packageName: String,
    val icon: Drawable,
    val uid: Int,
    val isSystemApp: Boolean,
    val isApp: Boolean,
    val isSharedUid: Boolean,
    val firstInstallTime: Long,
    val lastUpdateTime: Long,
) {
    val isShell: Boolean
        get() = packageName == SHELL_PACKAGE || uid == SHELL_UID

    companion object {
        const val SHELL_PACKAGE = "com.android.shell"
        const val SHELL_UID = 2000
    }
}

data class SuGrantAppState(
    val info: SuGrantAppInfo,
    val isGranted: Boolean,
    val initiallyGranted: Boolean = isGranted,
)

class SuperuserGrantViewModel(
    private val policyDB: PolicyDao
) : AsyncLoadViewModel() {

    private val _loading = MutableStateFlow(true)
    val loading: StateFlow<Boolean> = _loading.asStateFlow()

    private val _allApps = MutableStateFlow<List<SuGrantAppState>>(emptyList())

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

    var authenticate: (onSuccess: () -> Unit) -> Unit = { it() }

    val filteredApps: StateFlow<List<SuGrantAppState>> = combine(
        _allApps, _query, _showSystem, _showOS, _sortBy, _sortReverse
    ) { args ->
        @Suppress("UNCHECKED_CAST")
        val apps = args[0] as List<SuGrantAppState>
        val q = args[1] as String
        val showSys = args[2] as Boolean
        val showOS = args[3] as Boolean
        val sort = args[4] as SortBy
        val reverse = args[5] as Boolean

        val filtered = apps.filter { app ->
            val passFilter = app.isGranted ||
                app.initiallyGranted ||
                app.info.isShell ||
                ((showSys || !app.info.isSystemApp) &&
                ((showSys && showOS) || app.info.isApp))
            val passQuery = q.isBlank() ||
                app.info.label.contains(q, true) ||
                app.info.packageName.contains(q, true) ||
                (app.info.isShell && ("adb".startsWith(q.trim(), true) || q.trim().contains("adb", true)))
            passFilter && passQuery
        }

        val secondary: Comparator<SuGrantAppState> = when (sort) {
            SortBy.NAME -> compareBy<SuGrantAppState, String>(String.CASE_INSENSITIVE_ORDER) { it.info.label }
                .thenBy { it.info.packageName }
            SortBy.PACKAGE_NAME -> compareBy(String.CASE_INSENSITIVE_ORDER) { it.info.packageName }
            SortBy.INSTALL_TIME -> compareByDescending<SuGrantAppState> { it.info.firstInstallTime }
                .thenBy(String.CASE_INSENSITIVE_ORDER) { it.info.label }
                .thenBy { it.info.packageName }
            SortBy.UPDATE_TIME -> compareByDescending<SuGrantAppState> { it.info.lastUpdateTime }
                .thenBy(String.CASE_INSENSITIVE_ORDER) { it.info.label }
                .thenBy { it.info.packageName }
        }
        val comparator = compareBy<SuGrantAppState> { !it.initiallyGranted }
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

    fun toggleGrant(app: SuGrantAppState) {
        val willGrant = !app.isGranted
        fun updateState() {
            _allApps.update { apps ->
                apps.map { current ->
                    if (current.info.uid == app.info.uid) {
                        current.copy(isGranted = willGrant)
                    } else {
                        current
                    }
                }
            }
            viewModelScope.launch {
                try {
                    withContext(Dispatchers.IO) {
                        if (willGrant) {
                            val existing = policyDB.fetch(app.info.uid)
                            val suPolicy = existing ?: SuPolicy(app.info.uid)
                            suPolicy.policy = if (Config.suRestrict) SuPolicy.RESTRICT else SuPolicy.ALLOW
                            suPolicy.remain = 0L
                            policyDB.update(suPolicy)
                        } else {
                            policyDB.delete(app.info.uid)
                        }
                    }
                } catch (e: Exception) {
                    if (e is CancellationException) throw e
                    Timber.e(e)
                    _allApps.update { apps ->
                        apps.map { current ->
                            if (current.info.uid == app.info.uid && current.isGranted == willGrant) {
                                current.copy(isGranted = !willGrant)
                            } else {
                                current
                            }
                        }
                    }
                    return@launch
                }

                SuEvents.notifyPolicyChanged()

                val res = if (willGrant) CoreR.string.su_snack_grant else CoreR.string.su_snack_deny
                showSnackbar(AppContext.getString(res, app.info.label))
            }
        }

        if (Config.suAuth) {
            authenticate { updateState() }
        } else {
            updateState()
        }
    }

    @SuppressLint("InlinedApi")
    override suspend fun doLoadWork() {
        if (_allApps.value.isNotEmpty()) return
        _loading.value = true
        val apps = withContext(Dispatchers.Default) {
            val pm = AppContext.packageManager
            withContext(Dispatchers.IO) {
                policyDB.deleteOutdated()
            }
            val policies = withContext(Dispatchers.IO) {
                policyDB.fetchAll()
            }.associateBy { it.uid }

            val myUid = AppContext.applicationInfo.uid
            val myPackage = AppContext.packageName

            val installed = runCatching {
                pm.getInstalledApplications(MATCH_UNINSTALLED_PACKAGES)
            }.getOrElse { emptyList() }.toMutableList()

            if (installed.none { it.packageName == SuGrantAppInfo.SHELL_PACKAGE }) {
                runCatching {
                    pm.getApplicationInfo(SuGrantAppInfo.SHELL_PACKAGE, MATCH_UNINSTALLED_PACKAGES)
                }.recoverCatching {
                    pm.getApplicationInfo(SuGrantAppInfo.SHELL_PACKAGE, 0)
                }.recoverCatching {
                    pm.getPackageInfo(SuGrantAppInfo.SHELL_PACKAGE, MATCH_UNINSTALLED_PACKAGES).applicationInfo!!
                }.getOrNull()?.let { installed.add(it) }
            }

            installed
                .distinctBy { it.packageName }
                .asFlow()
                .filter {
                    it.packageName != myPackage &&
                    it.packageName != BuildConfig.APP_PACKAGE_NAME &&
                    it.uid != myUid
                }
                .concurrentMap { appInfo ->
                    val pkg = try {
                        pm.getPackageInfo(appInfo.packageName, MATCH_UNINSTALLED_PACKAGES)
                    } catch (_: Exception) {
                        null
                    }
                    val isSharedUid = pkg?.sharedUserId != null
                    val label = appInfo.getLabel(pm)
                    val icon = runCatching { appInfo.loadIcon(pm) }.getOrDefault(pm.defaultActivityIcon)
                    val isSystemApp = (appInfo.flags and ApplicationInfo.FLAG_SYSTEM) != 0
                    val isApp = ProcessCompat.isApplicationUid(appInfo.uid)
                    val suAppInfo = SuGrantAppInfo(
                        label = label,
                        packageName = appInfo.packageName,
                        icon = icon,
                        uid = appInfo.uid,
                        isSystemApp = isSystemApp,
                        isApp = isApp,
                        isSharedUid = isSharedUid,
                        firstInstallTime = pkg?.firstInstallTime ?: 0L,
                        lastUpdateTime = pkg?.lastUpdateTime ?: 0L,
                    )
                    val policy = policies[appInfo.uid]
                    val isGranted = policy != null && policy.policy >= SuPolicy.ALLOW
                    SuGrantAppState(
                        info = suAppInfo,
                        isGranted = isGranted,
                        initiallyGranted = isGranted,
                    )
                }
                .toCollection(ArrayList(installed.size))
                .apply {
                    sortWith(compareBy(
                        { !it.initiallyGranted },
                        { it.info.label.lowercase(Locale.ROOT) },
                        { it.info.packageName }
                    ))
                }
        }
        _allApps.value = apps
        _loading.value = false
    }
}
