package com.topjohnwu.magisk.ui.settings

import android.os.Build
import android.view.View
import android.widget.Toast
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.AppContext
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.activity
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.databinding.bindExtra
import com.topjohnwu.magisk.events.AddHomeIconEvent
import com.topjohnwu.magisk.events.BiometricEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch

class SettingsViewModel : BaseViewModel(), BaseSettingsItem.Handler {

    val items = createItems()
    val extraBindings = bindExtra {
        it.put(BR.handler, this)
    }

    init {
        viewModelScope.launch {
            Language.loadLanguages(this)
        }
    }

    private fun createItems(): List<BaseSettingsItem> {
        val context = AppContext
        val hidden = context.packageName != BuildConfig.APPLICATION_ID

        // Customization
        val list = mutableListOf(
            Customization,
            Theme, Language
        )
        if (isRunningAsStub && ShortcutManagerCompat.isRequestPinShortcutSupported(context))
            list.add(AddShortcut)

        // Manager
        list.addAll(listOf(
            AppSettings,
            UpdateChannel, UpdateChannelUrl, DoHToggle, UpdateChecker, DownloadPath
        ))
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1 &&
                Info.env.isActive && Const.USER_ID == 0) {
            if (hidden) list.add(Restore) else list.add(Hide)
        }

        // Magisk
        if (Info.env.isActive) {
            list.addAll(listOf(
                Magisk,
                SystemlessHosts
            ))
            if (Const.Version.atLeast_24_0()) {
                list.addAll(listOf(Zygisk, DenyList, DenyListConfig))
            }
        }

        // Superuser
        if (Info.showSuperUser) {
            list.addAll(listOf(
                Superuser,
                Tapjack, Biometrics, AccessMode, MultiuserMode, MountNamespaceMode,
                AutomaticResponse, RequestTimeout, SUNotification
            ))
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                // Biometric is only available on 6.0+
                list.remove(Biometrics)
            }
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
                // Re-authenticate is not feasible on 8.0+
                list.add(Reauthenticate)
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                // Can hide overlay windows on 12.0+
                list.remove(Tapjack)
            }
        }

        return list
    }

    override fun onItemPressed(view: View, item: BaseSettingsItem, andThen: () -> Unit) {
        when (item) {
            DownloadPath -> withExternalRW(andThen)
            UpdateChecker -> withPostNotificationPermission(andThen)
            Biometrics -> authenticate(andThen)
            Theme -> SettingsFragmentDirections.actionSettingsFragmentToThemeFragment().navigate()
            DenyListConfig -> SettingsFragmentDirections.actionSettingsFragmentToDenyFragment().navigate()
            SystemlessHosts -> createHosts()
            Hide, Restore -> withInstallPermission(andThen)
            AddShortcut -> AddHomeIconEvent().publish()
            else -> andThen()
        }
    }

    override fun onItemAction(view: View, item: BaseSettingsItem) {
        when (item) {
            UpdateChannel -> openUrlIfNecessary(view)
            is Hide -> viewModelScope.launch { HideAPK.hide(view.activity, item.value) }
            Restore -> viewModelScope.launch { HideAPK.restore(view.activity) }
            Zygisk -> if (Zygisk.mismatch) SnackbarEvent(R.string.reboot_apply_change).publish()
            else -> Unit
        }
    }

    private fun openUrlIfNecessary(view: View) {
        UpdateChannelUrl.refresh()
        if (UpdateChannelUrl.isEnabled && UpdateChannelUrl.value.isBlank()) {
            UpdateChannelUrl.onPressed(view, this)
        }
    }

    private fun authenticate(callback: () -> Unit) {
        BiometricEvent {
            // allow the change on success
            onSuccess { callback() }
        }.publish()
    }

    private fun createHosts() {
        Shell.cmd("add_hosts_module").submit {
            AppContext.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
        }
    }
}
