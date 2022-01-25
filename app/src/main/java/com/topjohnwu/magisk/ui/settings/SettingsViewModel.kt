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
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.databinding.adapterOf
import com.topjohnwu.magisk.databinding.itemBindingOf
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.events.AddHomeIconEvent
import com.topjohnwu.magisk.events.RecreateEvent
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.events.dialog.BiometricEvent
import com.topjohnwu.magisk.events.dialog.RestoreAppDialog
import com.topjohnwu.magisk.ktx.activity
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch

class SettingsViewModel : BaseViewModel(), BaseSettingsItem.Handler {

    val adapter = adapterOf<BaseSettingsItem>()
    val itemBinding = itemBindingOf<BaseSettingsItem> { it.bindExtra(BR.handler, this) }
    val items = createItems()

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
        if (Info.env.isActive) {
            if (Const.USER_ID == 0) {
                if (hidden) list.add(Restore) else list.add(Hide)
            }
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
        if (Utils.showSuperUser()) {
            list.addAll(listOf(
                Superuser,
                Tapjack, Biometrics, AccessMode, MultiuserMode, MountNamespaceMode,
                AutomaticResponse, RequestTimeout, SUNotification
            ))
            if (Build.VERSION.SDK_INT < 23) {
                // Biometric is only available on 6.0+
                list.remove(Biometrics)
            }
            if (Build.VERSION.SDK_INT < 26) {
                // Re-authenticate is not feasible on 8.0+
                list.add(Reauthenticate)
            }
            if (Build.VERSION.SDK_INT >= 31) {
                // Can hide overlay windows on 12.0+
                list.remove(Tapjack)
            }
        }

        return list
    }

    override fun onItemPressed(view: View, item: BaseSettingsItem, andThen: () -> Unit) {
        when (item) {
            DownloadPath -> withExternalRW(andThen)
            Biometrics -> authenticate(andThen)
            Theme -> SettingsFragmentDirections.actionSettingsFragmentToThemeFragment().navigate()
            DenyListConfig -> SettingsFragmentDirections.actionSettingsFragmentToDenyFragment().navigate()
            SystemlessHosts -> createHosts()
            Restore -> RestoreAppDialog().publish()
            AddShortcut -> AddHomeIconEvent().publish()
            else -> andThen()
        }
    }

    override fun onItemAction(view: View, item: BaseSettingsItem) {
        when (item) {
            Language -> RecreateEvent().publish()
            UpdateChannel -> openUrlIfNecessary(view)
            is Hide -> viewModelScope.launch { HideAPK.hide(view.activity, item.value) }
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
        Shell.su("add_hosts_module").submit {
            Utils.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
        }
    }
}
