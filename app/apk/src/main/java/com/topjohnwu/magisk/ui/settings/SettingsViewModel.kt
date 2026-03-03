package com.topjohnwu.magisk.ui.settings

import android.app.Activity
import android.widget.Toast
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.events.AddHomeIconEvent
import com.topjohnwu.magisk.events.AuthEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

class SettingsViewModel : BaseViewModel() {

    private val _denyListEnabled = MutableStateFlow(Config.denyList)
    val denyListEnabled: StateFlow<Boolean> = _denyListEnabled.asStateFlow()

    val zygiskMismatch get() = Config.zygisk != Info.isZygiskEnabled

    fun navigateToDenyList() {
        SettingsFragmentDirections.actionSettingsFragmentToDenyFragment().navigate()
    }

    fun requestAddShortcut() {
        AddHomeIconEvent().publish()
    }

    fun hideApp(activity: Activity, name: String) {
        viewModelScope.launch { AppMigration.hide(activity, name) }
    }

    fun restoreApp(activity: Activity) {
        viewModelScope.launch { AppMigration.restore(activity) }
    }

    fun createHosts() {
        viewModelScope.launch {
            RootUtils.addSystemlessHosts()
            AppContext.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
        }
    }

    fun toggleDenyList(enabled: Boolean) {
        _denyListEnabled.value = enabled
        val cmd = if (enabled) "enable" else "disable"
        Shell.cmd("magisk --denylist $cmd").submit { result ->
            if (result.isSuccess) {
                Config.denyList = enabled
            } else {
                _denyListEnabled.value = !enabled
            }
        }
    }

    fun withDownloadPathPermission(action: () -> Unit) = withExternalRW(action)

    fun withNotificationPermission(action: () -> Unit) = withPostNotificationPermission(action)

    fun withAuth(action: () -> Unit) = AuthEvent(action).publish()

    fun notifyZygiskChange() {
        if (zygiskMismatch) SnackbarEvent(R.string.reboot_apply_change).publish()
    }
}
