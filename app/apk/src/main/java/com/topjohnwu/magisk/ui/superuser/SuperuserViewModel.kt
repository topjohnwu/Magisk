package com.topjohnwu.magisk.ui.superuser

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.graphics.drawable.Drawable
import android.os.Process
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
import kotlinx.coroutines.withContext
import java.util.Locale

data class PolicyItem(
    val policy: SuPolicy,
    val packageName: String,
    val isSharedUid: Boolean,
    val icon: Drawable,
    val appName: String,
    val policyValue: Int = policy.policy,
    val notification: Boolean = policy.notification,
    val logging: Boolean = policy.logging,
) {
    val title get() = appName
    val isEnabled get() = policyValue >= SuPolicy.ALLOW
    val isRestricted get() = policyValue == SuPolicy.RESTRICT
}

class SuperuserViewModel(
    private val db: PolicyDao
) : AsyncLoadViewModel() {

    var authenticate: (onSuccess: () -> Unit) -> Unit = { it() }

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

    @SuppressLint("InlinedApi")
    override suspend fun doLoadWork() {
        if (!Info.showSuperUser) {
            _uiState.update { it.copy(loading = false) }
            return
        }
        _uiState.update { it.copy(loading = true) }
        withContext(Dispatchers.IO) {
            db.deleteOutdated()
            db.delete(AppContext.applicationInfo.uid)
            val policies = ArrayList<PolicyItem>()
            val pm = AppContext.packageManager
            for (policy in db.fetchAll()) {
                val pkgs =
                    if (policy.uid == Process.SYSTEM_UID) arrayOf("android")
                    else pm.getPackagesForUid(policy.uid)
                if (pkgs == null) {
                    db.delete(policy.uid)
                    continue
                }
                val map = pkgs.mapNotNull { pkg ->
                    try {
                        val info = pm.getPackageInfo(pkg, MATCH_UNINSTALLED_PACKAGES)
                        PolicyItem(
                            policy = policy,
                            packageName = info.packageName,
                            isSharedUid = info.sharedUserId != null,
                            icon = info.applicationInfo?.loadIcon(pm) ?: pm.defaultActivityIcon,
                            appName = info.applicationInfo?.getLabel(pm) ?: info.packageName,
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
            _uiState.update { it.copy(loading = false, policies = policies, suRestrict = Config.suRestrict) }
        }
    }

    fun refreshSuRestrict() {
        _uiState.update { it.copy(suRestrict = Config.suRestrict) }
    }

    val requiresAuth get() = Config.suAuth

    fun performDelete(item: PolicyItem, onDeleted: () -> Unit = {}) {
        viewModelScope.launch {
            db.delete(item.policy.uid)
            _uiState.update { state ->
                state.copy(policies = state.policies.filter { it.policy.uid != item.policy.uid })
            }
            onDeleted()
        }
    }

    fun updateNotify(item: PolicyItem) {
        val newNotification = !item.notification
        item.policy.notification = newNotification
        viewModelScope.launch {
            db.update(item.policy)
            _uiState.update { state ->
                state.copy(
                    policies = state.policies.map {
                        if (it.policy.uid == item.policy.uid) it.copy(notification = newNotification) else it
                    }
                )
            }
            val res = if (newNotification) R.string.su_snack_notif_on else R.string.su_snack_notif_off
            showSnackbar(AppContext.getString(res, item.appName))
        }
    }

    fun updateLogging(item: PolicyItem) {
        val newLogging = !item.logging
        item.policy.logging = newLogging
        viewModelScope.launch {
            db.update(item.policy)
            _uiState.update { state ->
                state.copy(
                    policies = state.policies.map {
                        if (it.policy.uid == item.policy.uid) it.copy(logging = newLogging) else it
                    }
                )
            }
            val res = if (newLogging) R.string.su_snack_log_on else R.string.su_snack_log_off
            showSnackbar(AppContext.getString(res, item.appName))
        }
    }

    fun updatePolicy(item: PolicyItem, newPolicy: Int) {
        fun updateState() {
            viewModelScope.launch {
                item.policy.policy = newPolicy
                db.update(item.policy)
                _uiState.update { state ->
                    state.copy(
                        policies = state.policies.map {
                            if (it.policy.uid == item.policy.uid) it.copy(policyValue = newPolicy) else it
                        }
                    )
                }
                val res = if (newPolicy >= SuPolicy.ALLOW) R.string.su_snack_grant else R.string.su_snack_deny
                showSnackbar(AppContext.getString(res, item.appName))
            }
        }

        if (Config.suAuth) {
            authenticate { updateState() }
        } else {
            updateState()
        }
    }

    fun togglePolicy(item: PolicyItem) {
        val newPolicy = if (item.isEnabled) SuPolicy.DENY else SuPolicy.ALLOW
        updatePolicy(item, newPolicy)
    }

    fun toggleRestrict(item: PolicyItem) {
        val newPolicy = if (item.isRestricted) SuPolicy.ALLOW else SuPolicy.RESTRICT
        updatePolicy(item, newPolicy)
    }
}
