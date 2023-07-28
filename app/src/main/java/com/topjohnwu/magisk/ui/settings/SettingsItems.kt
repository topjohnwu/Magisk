package com.topjohnwu.magisk.ui.settings

import android.content.Context
import android.content.res.Resources
import android.os.Build
import android.view.LayoutInflater
import android.view.View
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.activity
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.availableLocales
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.DialogSettingsAppNameBinding
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.utils.asText
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

// --- Customization

object Customization : BaseSettingsItem.Section() {
    override val title = R.string.settings_customization.asText()
}

object Language : BaseSettingsItem.Selector() {
    override var value
        get() = index
        set(value) {
            index = value
            Config.locale = entryValues[value]
        }

    override val title = R.string.language.asText()

    private var entries = emptyArray<String>()
    private var entryValues = emptyArray<String>()
    private var index = -1

    override fun entries(res: Resources) = entries
    override fun descriptions(res: Resources) = entries

    override fun onPressed(view: View, handler: Handler) {
        if (entries.isNotEmpty())
            super.onPressed(view, handler)
    }

    suspend fun loadLanguages(scope: CoroutineScope) {
        scope.launch {
            availableLocales().let { (names, values) ->
                entries = names
                entryValues = values
                val selectedLocale = currentLocale.getDisplayName(currentLocale)
                index = names.indexOfFirst { it == selectedLocale }.let { if (it == -1) 0 else it }
                notifyPropertyChanged(BR.description)
            }
        }
    }
}

object Theme : BaseSettingsItem.Blank() {
    override val icon = R.drawable.ic_paint
    override val title = R.string.section_theme.asText()
}

// --- App

object AppSettings : BaseSettingsItem.Section() {
    override val title = R.string.home_app_title.asText()
}

object Hide : BaseSettingsItem.Input() {
    override val title = R.string.settings_hide_app_title.asText()
    override val description = R.string.settings_hide_app_summary.asText()
    override var value = ""

    override val inputResult
        get() = if (isError) null else result

    @get:Bindable
    var result = "Settings"
        set(value) = set(value, field, { field = it }, BR.result, BR.error)

    val maxLength
        get() = HideAPK.MAX_LABEL_LENGTH

    @get:Bindable
    val isError
        get() = result.length > maxLength || result.isBlank()

    override fun getView(context: Context) = DialogSettingsAppNameBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object Restore : BaseSettingsItem.Blank() {
    override val title = R.string.settings_restore_app_title.asText()
    override val description = R.string.settings_restore_app_summary.asText()

    override fun onPressed(view: View, handler: Handler) {
        handler.onItemPressed(view, this) {
            MagiskDialog(view.activity).apply {
                setTitle(R.string.settings_restore_app_title)
                setMessage(R.string.restore_app_confirmation)
                setButton(MagiskDialog.ButtonType.POSITIVE) {
                    text = android.R.string.ok
                    onClick {
                        handler.onItemAction(view, this@Restore)
                    }
                }
                setButton(MagiskDialog.ButtonType.NEGATIVE) {
                    text = android.R.string.cancel
                }
                setCancelable(true)
                show()
            }
        }
    }
}

object AddShortcut : BaseSettingsItem.Blank() {
    override val title = R.string.add_shortcut_title.asText()
    override val description = R.string.setting_add_shortcut_summary.asText()
}

object DownloadPath : BaseSettingsItem.Input() {
    override var value
        get() = Config.downloadDir
        set(value) {
            Config.downloadDir = value
            notifyPropertyChanged(BR.description)
        }

    override val title = R.string.settings_download_path_title.asText()
    override val description get() = MediaStoreUtils.fullPath(value).asText()

    override var inputResult: String = value
        set(value) = set(value, field, { field = it }, BR.inputResult, BR.path)

    @get:Bindable
    val path get() = MediaStoreUtils.fullPath(inputResult)

    override fun getView(context: Context) = DialogSettingsDownloadPathBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChannel : BaseSettingsItem.Selector() {
    override var value
        get() = Config.updateChannel
        set(value) {
            Config.updateChannel = value
            Info.remote = Info.EMPTY_REMOTE
        }

    override val title = R.string.settings_update_channel_title.asText()

    override val entryRes = R.array.update_channel
    override fun entries(res: Resources): Array<String> {
        return super.entries(res).let {
            if (!Const.APP_IS_CANARY && !BuildConfig.DEBUG)
                it.copyOfRange(0, Config.Value.CANARY_CHANNEL)
            else it
        }
    }
}

object UpdateChannelUrl : BaseSettingsItem.Input() {
    override val title = R.string.settings_update_custom.asText()
    override val description get() = value.asText()
    override var value
        get() = Config.customChannelUrl
        set(value) {
            Config.customChannelUrl = value
            Info.remote = Info.EMPTY_REMOTE
            notifyPropertyChanged(BR.description)
        }

    override var inputResult: String = value
        set(value) = set(value, field, { field = it }, BR.inputResult)

    override fun refresh() {
        isEnabled = UpdateChannel.value == Config.Value.CUSTOM_CHANNEL
    }

    override fun getView(context: Context) = DialogSettingsUpdateChannelBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChecker : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_check_update_title.asText()
    override val description = R.string.settings_check_update_summary.asText()
    override var value by Config::checkUpdate
}

object DoHToggle : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_doh_title.asText()
    override val description = R.string.settings_doh_description.asText()
    override var value by Config::doh
}

object SystemlessHosts : BaseSettingsItem.Blank() {
    override val title = R.string.settings_hosts_title.asText()
    override val description = R.string.settings_hosts_summary.asText()
}

// --- Magisk

object Magisk : BaseSettingsItem.Section() {
    override val title = R.string.magisk.asText()
}

object Zygisk : BaseSettingsItem.Toggle() {
    override val title = R.string.zygisk.asText()
    override val description get() =
        if (mismatch) R.string.reboot_apply_change.asText()
        else R.string.settings_zygisk_summary.asText()
    override var value
        get() = Config.zygisk
        set(value) {
            Config.zygisk = value
            DenyList.isEnabled = value
            DenyListConfig.isEnabled = value
            notifyPropertyChanged(BR.description)
            DenyList.notifyPropertyChanged(BR.description)
        }
    val mismatch get() = value != Info.isZygiskEnabled
}

object DenyList : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_denylist_title.asText()
    override val description get() =
        if (isEnabled) {
            if (Zygisk.mismatch)
                R.string.reboot_apply_change.asText()
            else
                R.string.settings_denylist_summary.asText()
        } else {
            R.string.settings_denylist_error.asText(R.string.zygisk.asText())
        }

    override var value = Config.denyList
        set(value) {
            field = value
            val cmd = if (value) "enable" else "disable"
            Shell.cmd("magisk --denylist $cmd").submit { result ->
                if (result.isSuccess) {
                    Config.denyList = value
                } else {
                    field = !value
                    notifyPropertyChanged(BR.checked)
                }
            }
        }

    override fun refresh() {
        isEnabled = Zygisk.value
    }
}

object DenyListConfig : BaseSettingsItem.Blank() {
    override val title = R.string.settings_denylist_config_title.asText()
    override val description = R.string.settings_denylist_config_summary.asText()
    override fun refresh() {
        isEnabled = Zygisk.value
    }
}

// --- Superuser

object Tapjack : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_tapjack_title.asText()
    override val description = R.string.settings_su_tapjack_summary.asText()
    override var value by Config::suTapjack
}

object Biometrics : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_biometric_title.asText()
    override var description = R.string.settings_su_biometric_summary.asText()
    override var value
        get() = ServiceLocator.biometrics.isEnabled
        set(value) {
            Config.suBiometric = value
        }

    override fun refresh() {
        isEnabled = ServiceLocator.biometrics.isSupported
        if (!isEnabled) {
            description = R.string.no_biometric.asText()
        }
    }
}

object Superuser : BaseSettingsItem.Section() {
    override val title = R.string.superuser.asText()
}

object AccessMode : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_access.asText()
    override val entryRes = R.array.su_access
    override var value by Config::rootMode
}

object MultiuserMode : BaseSettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asText()
    override val entryRes = R.array.multiuser_mode
    override val descriptionRes = R.array.multiuser_summary
    override var value by Config::suMultiuserMode

    override fun refresh() {
        isEnabled = Const.USER_ID == 0
    }
}

object MountNamespaceMode : BaseSettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asText()
    override val entryRes = R.array.namespace
    override val descriptionRes = R.array.namespace_summary
    override var value by Config::suMntNamespaceMode
}

object AutomaticResponse : BaseSettingsItem.Selector() {
    override val title = R.string.auto_response.asText()
    override val entryRes = R.array.auto_response
    override var value by Config::suAutoResponse
}

object RequestTimeout : BaseSettingsItem.Selector() {
    override val title = R.string.request_timeout.asText()
    override val entryRes = R.array.request_timeout

    private val entryValues = listOf(10, 15, 20, 30, 45, 60)
    override var value = entryValues.indexOfFirst { it == Config.suDefaultTimeout }
        set(value) {
            field = value
            Config.suDefaultTimeout = entryValues[value]
        }
}

object SUNotification : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_notification.asText()
    override val entryRes = R.array.su_notification
    override var value by Config::suNotification
}

object Reauthenticate : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_reauth_title.asText()
    override val description = R.string.settings_su_reauth_summary.asText()
    override var value by Config::suReAuth

    override fun refresh() {
        isEnabled = Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Info.showSuperUser
    }
}
