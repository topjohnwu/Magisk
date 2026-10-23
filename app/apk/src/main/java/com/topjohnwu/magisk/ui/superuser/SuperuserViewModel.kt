package com.topjohnwu.magisk.ui.superuser

import android.content.pm.PackageManager
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.os.Process
import androidx.annotation.MainThread
import androidx.compose.runtime.Immutable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.data.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.ktx.getLabel
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.su.SuEvents
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.FlowPreview
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import java.util.Locale
import java.util.concurrent.ConcurrentHashMap

@Immutable
data class PolicyItem(
    val uid: Int,
    val packageName: String,
    val appName: String,
    val isSharedUid: Boolean,
    val policyValue: Int,
    val notification: Boolean,
    val logging: Boolean,
) {
    val title: String get() = appName
    val isEnabled: Boolean get() = policyValue >= SuPolicy.ALLOW
    val isRestricted: Boolean get() = policyValue == SuPolicy.RESTRICT
}

class SuperuserViewModel(
    private val db: PolicyDao
) : AsyncLoadViewModel() {

    // Serializes database access so writes land in the same order as the UI changes,
    // and reloads never interleave with them
    private val writeLock = Mutex()
    // Edits already applied to the UI state but not yet written to the database
    private val pendingWrites = ConcurrentHashMap<Int, SuPolicy>()

    init {
        @OptIn(FlowPreview::class)
        viewModelScope.launch {
            SuEvents.policyChanged.debounce(500).collect { reload() }
        }
    }

    data class UiState(
        val loading: Boolean = true,
        val policies: List<PolicyItem> = emptyList(),
        val suRestrict: Boolean = Config.suRestrict,
    )

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    override suspend fun doLoadWork() {
        if (!Info.showSuperUser) {
            _uiState.update { it.copy(loading = false) }
            return
        }
        _uiState.update { it.copy(loading = true) }
        writeLock.withLock {
            val loaded = withContext(Dispatchers.IO) {
                db.deleteOutdated()
                db.delete(AppContext.applicationInfo.uid)
                val policies = ArrayList<PolicyItem>()
                val pm = AppContext.packageManager
                for (policy in db.fetchAll()) {
                    val pkgs =
                        if (policy.uid == Process.SYSTEM_UID) arrayOf("android")
                        else pm.getPackagesForUid(PolicyDao.denormalizeUid(policy.uid))
                    if (pkgs == null) {
                        db.delete(policy.uid)
                        continue
                    }
                    val map = pkgs.mapNotNull { pkg ->
                        try {
                            val info = pm.getPackageInfo(pkg, MATCH_UNINSTALLED_PACKAGES)
                            PolicyItem(
                                uid = policy.uid,
                                packageName = info.packageName,
                                appName = info.applicationInfo?.getLabel(pm) ?: info.packageName,
                                isSharedUid = info.sharedUserId != null,
                                policyValue = policy.policy,
                                notification = policy.notification,
                                logging = policy.logging,
                            )
                        } catch (_: PackageManager.NameNotFoundException) {
                            null
                        }
                    }
                    if (map.isEmpty()) {
                        db.delete(policy.uid)
                        continue
                    }
                    policies.addAll(map)
                }
                policies.sortWith(compareBy(
                    { it.appName.lowercase(Locale.ROOT) },
                    { it.packageName }
                ))
                policies
            }
            // Publish on the main thread, where all edits are applied, so the update below
            // never races with them. Keep edits queued behind this reload so they don't revert.
            _uiState.update { state ->
                val merged = loaded.map { item ->
                    pendingWrites[item.uid]?.let {
                        item.copy(policyValue = it.policy, notification = it.notification, logging = it.logging)
                    } ?: item
                }
                state.copy(loading = false, policies = merged, suRestrict = Config.suRestrict)
            }
        }
    }

    fun refreshSuRestrict() {
        _uiState.update { it.copy(suRestrict = Config.suRestrict) }
    }

    fun performDelete(item: PolicyItem, onDeleted: () -> Unit = {}) {
        viewModelScope.launch {
            writeLock.withLock {
                db.delete(item.uid)
                pendingWrites.remove(item.uid)
                _uiState.update { state ->
                    state.copy(policies = state.policies.filter { it.uid != item.uid })
                }
            }
            onDeleted()
        }
    }

    /**
     * Apply [transform] to the latest state of every item with [uid], then persist the result.
     * Deriving each change from the current state (instead of the item captured by the UI) and
     * writing in order ensures quick consecutive edits cannot overwrite each other.
     */
    @MainThread
    private fun updateItem(uid: Int, transform: (PolicyItem) -> PolicyItem): PolicyItem? {
        // All state updates happen on the main thread, so this update is never retried
        var updated: PolicyItem? = null
        _uiState.update { state ->
            state.copy(
                policies = state.policies.map {
                    if (it.uid == uid) transform(it).also { new -> updated = new } else it
                }
            )
        }
        val item = updated ?: return null
        val policy = SuPolicy(
            uid = item.uid,
            policy = item.policyValue,
            notification = item.notification,
            logging = item.logging,
        )
        pendingWrites[uid] = policy
        viewModelScope.launch {
            try {
                writeLock.withLock {
                    // Skip if the policy was deleted while this write was queued
                    if (_uiState.value.policies.any { it.uid == uid }) db.update(policy)
                }
            } finally {
                // Use the original key: db.update() normalizes the uid of the policy object
                pendingWrites.remove(uid, policy)
            }
        }
        return item
    }

    fun updateNotify(item: PolicyItem) {
        val updated = updateItem(item.uid) { it.copy(notification = !it.notification) } ?: return
        val res = if (updated.notification) R.string.su_snack_notif_on else R.string.su_snack_notif_off
        showToast(AppContext.getString(res, item.appName))
    }

    fun updateLogging(item: PolicyItem) {
        val updated = updateItem(item.uid) { it.copy(logging = !it.logging) } ?: return
        val res = if (updated.logging) R.string.su_snack_log_on else R.string.su_snack_log_off
        showToast(AppContext.getString(res, item.appName))
    }

    fun togglePolicy(item: PolicyItem) {
        updateItem(item.uid) {
            it.copy(policyValue = if (it.isEnabled) SuPolicy.DENY else SuPolicy.ALLOW)
        }
    }

    fun toggleRestrict(item: PolicyItem) {
        updateItem(item.uid) {
            it.copy(policyValue = if (it.isRestricted) SuPolicy.ALLOW else SuPolicy.RESTRICT)
        }
    }
}
