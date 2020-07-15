package com.topjohnwu.magisk.ui.settings

import android.content.Context
import android.os.Build
import android.os.Environment
import android.view.LayoutInflater
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.core.utils.availableLocales
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.DialogSettingsAppNameBinding
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.model.entity.recycler.SettingsItem
import com.topjohnwu.magisk.utils.asTransitive
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import java.io.File

// --- Customization

object Customization : SettingsItem.Section() {
    override val title = R.string.settings_customization.asTransitive()
}

object Language : SettingsItem.Selector() {
    override var value = -1
        set(value) = setS(value, field, { field = it }) {
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

object Theme : SettingsItem.Blank() {
    override val icon = R.drawable.ic_paint
    override val title = R.string.section_theme.asTransitive()
}

// --- Manager

object Manager : SettingsItem.Section() {
    override val title = R.string.manager.asTransitive()
}

object ClearRepoCache : SettingsItem.Blank() {
    override val title = R.string.settings_clear_cache_title.asTransitive()
    override val description = R.string.settings_clear_cache_summary.asTransitive()

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
}

object Hide : SettingsItem.Input() {
    override val title = R.string.settings_hide_manager_title.asTransitive()
    override val description = R.string.settings_hide_manager_summary.asTransitive()
    override val showStrip = false
    override var value = resources.getString(R.string.re_app_name)
        set(value) = setV(value, field, { field = it }, BR.error)

    @get:Bindable
    val isError get() = value.length > 14 || value.isBlank()

    override val intermediate: String?
        get() = if (isError) null else value

    override fun getView(context: Context) = DialogSettingsAppNameBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object Restore : SettingsItem.Blank() {
    override val title = R.string.settings_restore_manager_title.asTransitive()
    override val description = R.string.settings_restore_manager_summary.asTransitive()
}

@Suppress("FunctionName")
fun HideOrRestore() =
    if (get<Context>().packageName == BuildConfig.APPLICATION_ID) Hide else Restore

object DownloadPath : SettingsItem.Input() {
    override var value = Config.downloadPath
        set(value) = setV(value, field, { field = it }) { Config.downloadPath = it }
    override val title = R.string.settings_download_path_title.asTransitive()
    override val intermediate: String?
        get() = if (Utils.ensureDownloadPath(result) != null) result else null

    @get:Bindable
    var result = value
        set(value) = set(value, field, { field = it }, BR.result, BR.path)

    @get:Bindable
    val path get() = File(Environment.getExternalStorageDirectory(), result).absolutePath.orEmpty()

    override fun getView(context: Context) = DialogSettingsDownloadPathBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChannel : SettingsItem.Selector() {
    override var value  = Config.updateChannel
        set(value) = setS(value, field, { field = it }) { Config.updateChannel = it }

    override val title = R.string.settings_update_channel_title.asTransitive()
    override val entries get() = resources.getStringArray(R.array.update_channel).let {
        if (BuildConfig.DEBUG) it.toMutableList().apply { add("Canary") }.toTypedArray() else it
    }
    override val entryValRes = R.array.value_array
}

object UpdateChannelUrl : SettingsItem.Input() {
    override val title = R.string.settings_update_custom.asTransitive()
    override var value = Config.customChannelUrl
        set(value) = setV(value, field, { field = it }) { Config.customChannelUrl = it }
    override val intermediate: String? get() = result

    @get:Bindable
    var result = value
        set(value) = set(value, field, { field = it }, BR.result)

    override fun refresh() {
        isEnabled = UpdateChannel.value == Config.Value.CUSTOM_CHANNEL
    }

    override fun getView(context: Context) = DialogSettingsUpdateChannelBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChecker : SettingsItem.Toggle() {
    override val title = R.string.settings_check_update_title.asTransitive()
    override val description = R.string.settings_check_update_summary.asTransitive()
    override var value = Config.checkUpdate
        set(value) = setV(value, field, { field = it }) {
            Config.checkUpdate = it
            Utils.scheduleUpdateCheck(get())
        }
}

// check whether is module already installed beforehand?
object SystemlessHosts : SettingsItem.Blank() {
    override val title = R.string.settings_hosts_title.asTransitive()
    override val description = R.string.settings_hosts_summary.asTransitive()
}

object Biometrics : SettingsItem.Toggle() {
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

object Reauthenticate : SettingsItem.Toggle() {
    override val title = R.string.settings_su_reauth_title.asTransitive()
    override val description = R.string.settings_su_reauth_summary.asTransitive()
    override var value = Config.suReAuth
        set(value) = setV(value, field, { field = it }) { Config.suReAuth = it }

    override fun refresh() {
        isEnabled = Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Utils.showSuperUser()
    }
}

// --- Magisk

object Magisk : SettingsItem.Section() {
    override val title = R.string.magisk.asTransitive()
}

object MagiskHide : SettingsItem.Toggle() {
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

object Superuser : SettingsItem.Section() {
    override val title = R.string.superuser.asTransitive()
}

object AccessMode : SettingsItem.Selector() {
    override val title = R.string.superuser_access.asTransitive()
    override val entryRes = R.array.su_access
    override val entryValRes = R.array.value_array

    override var value = Config.rootMode
        set(value) = setS(value, field, { field = it }) {
            Config.rootMode = entryValues[it].toInt()
        }
}

object MultiuserMode : SettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asTransitive()
    override val entryRes = R.array.multiuser_mode
    override val entryValRes = R.array.value_array

    override var value = Config.suMultiuserMode
        set(value) = setS(value, field, { field = it }) {
            Config.suMultiuserMode = entryValues[it].toInt()
        }

    override val description
        get() = resources.getStringArray(R.array.multiuser_summary)[value].asTransitive()

    override fun refresh() {
        isEnabled = Const.USER_ID == 0
    }
}

object MountNamespaceMode : SettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asTransitive()
    override val entryRes = R.array.namespace
    override val entryValRes = R.array.value_array

    override var value = Config.suMntNamespaceMode
        set(value) = setS(value, field, { field = it }) {
            Config.suMntNamespaceMode = entryValues[it].toInt()
        }

    override val description
        get() = resources.getStringArray(R.array.namespace_summary)[value].asTransitive()
}

object AutomaticResponse : SettingsItem.Selector() {
    override val title = R.string.auto_response.asTransitive()
    override val entryRes = R.array.auto_response
    override val entryValRes = R.array.value_array

    override var value = Config.suAutoReponse
        set(value) = setS(value, field, { field = it }) {
            Config.suAutoReponse = entryValues[it].toInt()
        }
}

object RequestTimeout : SettingsItem.Selector() {
    override val title = R.string.request_timeout.asTransitive()
    override val entryRes = R.array.request_timeout
    override val entryValRes = R.array.request_timeout_value

    override var value = selected
        set(value) = setS(value, field, { field = it }) {
            Config.suDefaultTimeout = entryValues[it].toInt()
        }

    private val selected: Int
        get() = entryValues.indexOfFirst { it.toInt() == Config.suDefaultTimeout }
}

object SUNotification : SettingsItem.Selector() {
    override val title = R.string.superuser_notification.asTransitive()
    override val entryRes = R.array.su_notification
    override val entryValRes = R.array.value_array

    override var value = Config.suNotification
        set(value) = setS(value, field, { field = it }) {
            Config.suNotification = entryValues[it].toInt()
        }
}
