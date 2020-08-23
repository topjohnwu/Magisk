package com.topjohnwu.magisk.ui.settings

import android.content.Context
import android.os.Build
import android.view.LayoutInflater
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.UpdateCheckService
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.availableLocales
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.DialogSettingsAppNameBinding
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.asTransitive
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

// --- Customization

object Customization : BaseSettingsItem.Section() {
    override val title = R.string.settings_customization.asTransitive()
}

object Language : BaseSettingsItem.Selector() {
    override var value = -1
        set(value) = setV(value, field, { field = it }) {
            Config.locale = entryValues[it]
        }

    override val title = R.string.language.asTransitive()
    override var entries = emptyArray<String>()
    override var entryValues = emptyArray<String>()

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
    override val title = R.string.section_theme.asTransitive()
}

// --- Manager

object Manager : BaseSettingsItem.Section() {
    override val title = R.string.manager.asTransitive()
}

object ClearRepoCache : BaseSettingsItem.Blank() {
    override val title = R.string.settings_clear_cache_title.asTransitive()
    override val description = R.string.settings_clear_cache_summary.asTransitive()

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
}

object Hide : BaseSettingsItem.Input() {
    override val title = R.string.settings_hide_manager_title.asTransitive()
    override val description = R.string.settings_hide_manager_summary.asTransitive()

    override var value = ""
        set(value) = setV(value, field, { field = it })

    override val inputResult
        get() = if (isError) null else result

    @get:Bindable
    var result = "Manager"
        set(value) = set(value, field, { field = it }, BR.result, BR.error)

    @get:Bindable
    val isError
        get() = result.length > 14 || result.isBlank()

    override fun getView(context: Context) = DialogSettingsAppNameBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root

}

object Restore : BaseSettingsItem.Blank() {
    override val title = R.string.settings_restore_manager_title.asTransitive()
    override val description = R.string.settings_restore_manager_summary.asTransitive()
}

object AddShortcut : BaseSettingsItem.Blank() {
    override val title = R.string.add_shortcut_title.asTransitive()
    override val description = R.string.setting_add_shortcut_summary.asTransitive()
}

object DownloadPath : BaseSettingsItem.Input() {
    override var value = Config.downloadDir
        set(value) = setV(value, field, { field = it }) { Config.downloadDir = it }

    override val title = R.string.settings_download_path_title.asTransitive()
    override val description get() = path.asTransitive()

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
    override var value = Config.updateChannel
        set(value) = setV(value, field, { field = it }) { Config.updateChannel = it }

    override val title = R.string.settings_update_channel_title.asTransitive()
    override val entries
        get() = resources.getStringArray(R.array.update_channel).let {
            if (BuildConfig.DEBUG) it.toMutableList().apply { add("Canary") }.toTypedArray() else it
        }
    override val entryValRes = R.array.value_array
}

object UpdateChannelUrl : BaseSettingsItem.Input() {
    override val title = R.string.settings_update_custom.asTransitive()
    override var value = Config.customChannelUrl
        set(value) = setV(value, field, { field = it }) { Config.customChannelUrl = it }
    override val description get() = value.asTransitive()

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
    override val title = R.string.settings_check_update_title.asTransitive()
    override val description = R.string.settings_check_update_summary.asTransitive()
    override var value = Config.checkUpdate
        set(value) = setV(value, field, { field = it }) {
            Config.checkUpdate = it
            UpdateCheckService.schedule(get())
        }
}

// check whether is module already installed beforehand?
object SystemlessHosts : BaseSettingsItem.Blank() {
    override val title = R.string.settings_hosts_title.asTransitive()
    override val description = R.string.settings_hosts_summary.asTransitive()
}

object Biometrics : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_biometric_title.asTransitive()
    override var value = Config.suBiometric
        set(value) = setV(value, field, { field = it }) { Config.suBiometric = it }
    override var description = R.string.settings_su_biometric_summary.asTransitive()

    override fun refresh() {
        isEnabled = BiometricHelper.isSupported
        if (!isEnabled) {
            value = false
            description = R.string.no_biometric.asTransitive()
        }
    }
}

object Reauthenticate : BaseSettingsItem.Toggle() {
    override val title = R.string.settings_su_reauth_title.asTransitive()
    override val description = R.string.settings_su_reauth_summary.asTransitive()
    override var value = Config.suReAuth
        set(value) = setV(value, field, { field = it }) { Config.suReAuth = it }

    override fun refresh() {
        isEnabled = Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Utils.showSuperUser()
    }
}

// --- Magisk

object Magisk : BaseSettingsItem.Section() {
    override val title = R.string.magisk.asTransitive()
}

object MagiskHide : BaseSettingsItem.Toggle() {
    override val title = R.string.magiskhide.asTransitive()
    override val description = R.string.settings_magiskhide_summary.asTransitive()
    override var value = Config.magiskHide
        set(value) = setV(value, field, { field = it }) {
            Config.magiskHide = it
            when {
                it -> Shell.su("magiskhide --enable").submit()
                else -> Shell.su("magiskhide --disable").submit()
            }
        }
}

// --- Superuser

object Superuser : BaseSettingsItem.Section() {
    override val title = R.string.superuser.asTransitive()
}

object AccessMode : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_access.asTransitive()
    override val entryRes = R.array.su_access
    override val entryValRes = R.array.value_array

    override var value = Config.rootMode
        set(value) = setV(value, field, { field = it }) {
            Config.rootMode = entryValues[it].toInt()
        }
}

object MultiuserMode : BaseSettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asTransitive()
    override val entryRes = R.array.multiuser_mode
    override val entryValRes = R.array.value_array

    override var value = Config.suMultiuserMode
        set(value) = setV(value, field, { field = it }) {
            Config.suMultiuserMode = entryValues[it].toInt()
        }

    override val description
        get() = resources.getStringArray(R.array.multiuser_summary)[value].asTransitive()

    override fun refresh() {
        isEnabled = Const.USER_ID == 0
    }
}

object MountNamespaceMode : BaseSettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asTransitive()
    override val entryRes = R.array.namespace
    override val entryValRes = R.array.value_array

    override var value = Config.suMntNamespaceMode
        set(value) = setV(value, field, { field = it }) {
            Config.suMntNamespaceMode = entryValues[it].toInt()
        }

    override val description
        get() = resources.getStringArray(R.array.namespace_summary)[value].asTransitive()
}

object AutomaticResponse : BaseSettingsItem.Selector() {
    override val title = R.string.auto_response.asTransitive()
    override val entryRes = R.array.auto_response
    override val entryValRes = R.array.value_array

    override var value = Config.suAutoReponse
        set(value) = setV(value, field, { field = it }) {
            Config.suAutoReponse = entryValues[it].toInt()
        }
}

object RequestTimeout : BaseSettingsItem.Selector() {
    override val title = R.string.request_timeout.asTransitive()
    override val entryRes = R.array.request_timeout
    override val entryValRes = R.array.request_timeout_value

    override var value = selected
        set(value) = setV(value, field, { field = it }) {
            Config.suDefaultTimeout = entryValues[it].toInt()
        }

    private val selected: Int
        get() = entryValues.indexOfFirst { it.toInt() == Config.suDefaultTimeout }
}

object SUNotification : BaseSettingsItem.Selector() {
    override val title = R.string.superuser_notification.asTransitive()
    override val entryRes = R.array.su_notification
    override val entryValRes = R.array.value_array

    override var value = Config.suNotification
        set(value) = setV(value, field, { field = it }) {
            Config.suNotification = entryValues[it].toInt()
        }
}
