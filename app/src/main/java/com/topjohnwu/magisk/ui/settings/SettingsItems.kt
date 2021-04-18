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
import com.topjohnwu.magisk.core.UpdateCheckService
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.availableLocales
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.DialogSettingsAppNameBinding
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.asText
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

// --- Customization

object Customization : BaseSettingsItem.Section() {
    override val title = R.string.settings_customization.asText()
}

object Language : BaseSettingsItem.Selector() {
    override var value = -1
        set(value) = setV(value, field, { field = it }) {
            Config.locale = entryValues[it]
        }

    override val title = R.string.language.asText()

    private var entries = emptyArray<String>()
    private var entryValues = emptyArray<String>()

    override fun entries(res: Resources) = entries
    override fun descriptions(res: Resources) = entries

    override fun onPressed(view: View, callback: Callback) {
        if (entries.isEmpty()) return
        super.onPressed(view, callback)
    }

    suspend fun loadLanguages(scope: CoroutineScope) {
        scope.launch {
            availableLocales().let { (names, values) ->
                entries = names
                entryValues = values
                val selectedLocale = currentLocale.getDisplayName(currentLocale)
                value = names.indexOfFirst { it == selectedLocale }.let { if (it == -1) 0 else it }
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

object ClearRepoCache : BaseSettingsItem.Blank() {
    override val title = R.string.settings_clear_cache_title.asText()
    override val description = R.string.settings_clear_cache_summary.asText()

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
}

object Hide : BaseSettingsItem.Input() {
    override val title = R.string.settings_hide_app_title.asText()
    override val description = R.string.settings_hide_app_summary.asText()

    override var value = ""
        set(value) = setV(value, field, { field = it })

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

    override fun refresh() {
        isEnabled = Info.remote.stub.versionCode > 0
    }
}

object Restore : BaseSettingsItem.Blank() {
    override val title = R.string.settings_restore_app_title.asText()
    override val description = R.string.settings_restore_app_summary.asText()
}

object AddShortcut : BaseSettingsItem.Blank() {
    override val title = R.string.add_shortcut_title.asText()
    override val description = R.string.setting_add_shortcut_summary.asText()
}

object DownloadPath : BaseSettingsItem.Input() {
    override var value = Config.downloadDir
        set(value) = setV(value, field, { field = it }) { Config.downloadDir = it }

    override val title = R.string.settings_download_path_title.asText()
    override val description get() = path.asText()

    override val inputResult: String get() = result

    @get:Bindable
    var result = value
        set(value) = set(value, field, { field = it }, BR.result, BR.path)

    @get:Bindable
    val path
        get() = MediaStoreUtils.fullPath(result)

    override fun getView(context: Context) = DialogSettingsDownloadPathBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChannel : BaseSettingsItem.Selector() {
    override var value = Config.updateChannel.let { if (it < 0) 0 else it }
        set(value) = setV(value, field, { field = it }) {
            Config.updateChannel = it
            Info.remote = Info.EMPTY_REMOTE
        }

    override val title = R.string.settings_update_channel_title.asText()

    override val entryRes = R.array.update_channel
    override fun entries(res: Resources): Array<String> {
        return super.entries(res).let {
            if (!BuildConfig.DEBUG)
                it.copyOfRange(0, Config.Value.CANARY_CHANNEL)
            else it
        }
    }
}

object UpdateChannelUrl : BaseSettingsItem.Input() {
    override val title = R.string.settings_update_custom.asText()
    override var value = Config.customChannelUrl
        set(value) = setV(value, field, { field = it }) {
            Config.customChannelUrl = it
            Info.remote = Info.EMPTY_REMOTE
        }
    override val description get() = value.asText()

    override val inputResult get() = result

    @get:Bindable
    var result = value
        set(value) = set(value, field, { field = it }, BR.result)

    override fun refresh() {
        isEnabled = UpdateChannel.value == Config.Value.CUSTOM_CHANNEL
    }

    override fun getView(context: Context) = DialogSettingsUpdateChannelBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChecker : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_check_update_title.asText()
    override val description = R.string.settings_check_update_summary.asText()
    override var value = Config.checkUpdate
        set(value) = setV(value, field, { field = it }) {
            Config.checkUpdate = it
            UpdateCheckService.schedule(AppContext)
        }
}

object DoHToggle : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_doh_title.asText()
    override val description = R.string.settings_doh_description.asText()
    override var value = Config.doh
        set(value) = setV(value, field, { field = it }) {
            Config.doh = it
        }
}

// check whether is module already installed beforehand?
object SystemlessHosts : BaseSettingsItem.Blank() {
    override val title = R.string.settings_hosts_title.asText()
    override val description = R.string.settings_hosts_summary.asText()
}

object Tapjack : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_tapjack_title.asText()
    override var description = R.string.settings_su_tapjack_summary.asText()
    override var value = Config.suTapjack
        set(value) = setV(value, field, { field = it }) { Config.suTapjack = it }
}

object Biometrics : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_biometric_title.asText()
    override var value = Config.suBiometric
        set(value) = setV(value, field, { field = it }) { Config.suBiometric = it }
    override var description = R.string.settings_su_biometric_summary.asText()

    override fun refresh() {
        isEnabled = BiometricHelper.isSupported
        if (!isEnabled) {
            value = false
            description = R.string.no_biometric.asText()
        }
    }
}

object Reauthenticate : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_reauth_title.asText()
    override val description = R.string.settings_su_reauth_summary.asText()
    override var value = Config.suReAuth
        set(value) = setV(value, field, { field = it }) { Config.suReAuth = it }

    override fun refresh() {
        isEnabled = Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Utils.showSuperUser()
    }
}

// --- Magisk

object Magisk : BaseSettingsItem.Section() {
    override val title = R.string.magisk.asText()
}

object MagiskHide : BaseSettingsItem.Toggle() {
    override val title = R.string.magiskhide.asText()
    override val description = R.string.settings_magiskhide_summary.asText()
    override var value = Config.magiskHide
        set(value) = setV(value, field, { field = it }) {
            val cmd = if (it) "enable" else "disable"
            Shell.su("magiskhide $cmd").submit { cb ->
                if (cb.isSuccess) Config.magiskHide = it
                else field = !it
            }
        }
}

// --- Superuser

object Superuser : BaseSettingsItem.Section() {
    override val title = R.string.superuser.asText()
}

object AccessMode : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_access.asText()
    override val entryRes = R.array.su_access

    override var value = Config.rootMode
        set(value) = setV(value, field, { field = it }) {
            Config.rootMode = it
        }
}

object MultiuserMode : BaseSettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asText()
    override val entryRes = R.array.multiuser_mode
    override val descriptionRes = R.array.multiuser_summary

    override var value = Config.suMultiuserMode
        set(value) = setV(value, field, { field = it }) {
            Config.suMultiuserMode = it
        }

    override fun refresh() {
        isEnabled = Const.USER_ID == 0
    }
}

object MountNamespaceMode : BaseSettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asText()
    override val entryRes = R.array.namespace
    override val descriptionRes = R.array.namespace_summary

    override var value = Config.suMntNamespaceMode
        set(value) = setV(value, field, { field = it }) {
            Config.suMntNamespaceMode = it
        }
}

object AutomaticResponse : BaseSettingsItem.Selector() {
    override val title = R.string.auto_response.asText()
    override val entryRes = R.array.auto_response

    override var value = Config.suAutoResponse
        set(value) = setV(value, field, { field = it }) {
            Config.suAutoResponse = it
        }
}

object RequestTimeout : BaseSettingsItem.Selector() {
    override val title = R.string.request_timeout.asText()
    override val entryRes = R.array.request_timeout

    private val entryValues = listOf(10, 15, 20, 30, 45, 60)
    override var value = selected
        set(value) = setV(value, field, { field = it }) {
            Config.suDefaultTimeout = entryValues[it]
        }

    private val selected: Int
        get() = entryValues.indexOfFirst { it == Config.suDefaultTimeout }
}

object SUNotification : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_notification.asText()
    override val entryRes = R.array.su_notification

    override var value = Config.suNotification
        set(value) = setV(value, field, { field = it }) {
            Config.suNotification = it
        }
}
