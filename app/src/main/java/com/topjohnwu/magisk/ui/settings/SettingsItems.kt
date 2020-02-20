package com.topjohnwu.magisk.ui.settings

import android.content.Context
import android.os.Build
import android.os.Environment
import android.view.LayoutInflater
import android.widget.Toast
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isCanaryVersion
import com.topjohnwu.magisk.core.utils.*
import com.topjohnwu.magisk.databinding.DialogSettingsAppNameBinding
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.recycler.SettingsItem
import com.topjohnwu.magisk.utils.asTransitive
import com.topjohnwu.superuser.Shell
import java.io.File
import kotlin.math.max
import kotlin.math.min

// --- Customization

object Customization : SettingsItem.Section() {
    override val title = R.string.settings_customization.asTransitive()
}

object Language : SettingsItem.Selector() {
    override var value by bindableValue(0) {
        Config.locale = entryValues[it]
        refreshLocale()
    }

    override val title = R.string.language.asTransitive()
    override var entries = emptyArray<String>()
    override var entryValues = emptyArray<String>()

    init {
        availableLocales.subscribeK { (names, values) ->
            entries = names
            entryValues = values
            val selectedLocale = currentLocale.getDisplayName(currentLocale)
            value = names.indexOfFirst { it == selectedLocale }.let { if (it == -1) 0 else it }
            notifyChange(BR.selectedEntry)
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
    override var value: String = resources.getString(R.string.re_app_name)
        set(value) {
            field = value
            notifyChange(BR.value)
            notifyChange(BR.error)
        }

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
    override var value: String by bindableValue(Config.downloadPath) { Config.downloadPath = it }
    override val title = R.string.settings_download_path_title.asTransitive()
    override val intermediate: String?
        get() = if (Utils.ensureDownloadPath(result) != null) result else null

    @get:Bindable
    var result = value
        set(value) {
            field = value
            notifyChange(BR.result)
            notifyChange(BR.path)
        }

    @get:Bindable
    val path
        get() = File(Environment.getExternalStorageDirectory(), result).absolutePath.orEmpty()

    override fun getView(context: Context) = DialogSettingsDownloadPathBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object GridSize : SettingsItem.Selector() {
    override var value by bindableValue(Config.listSpanCount - 1) {
        Config.listSpanCount = max(1, min(2, it + 1))
    }

    override val title = R.string.settings_grid_column_count_title.asTransitive()
    override val description = R.string.settings_grid_column_count_summary.asTransitive()
    override val entryRes = R.array.span_count
    override val entryValRes = R.array.value_array
}

object UpdateChannel : SettingsItem.Selector() {
    override var value by bindableValue(Config.updateChannel) { Config.updateChannel = it }

    override val title = R.string.settings_update_channel_title.asTransitive()
    override val entries get() = resources.getStringArray(R.array.update_channel).let {
        if (!isCanaryVersion && Config.updateChannel < Config.Value.CANARY_CHANNEL)
            it.take(it.size - 2).toTypedArray() else it
    }
    override val entryValRes = R.array.value_array
}

object UpdateChannelUrl : SettingsItem.Input() {
    override val title = R.string.settings_update_custom.asTransitive()
    override var value by bindableValue(Config.customChannelUrl) { Config.customChannelUrl = it }
    override val intermediate: String? get() = result

    @get:Bindable
    var result = value
        set(value) {
            field = value
            notifyChange(BR.result)
        }

    override fun refresh() {
        isEnabled = UpdateChannel.value == Config.Value.CUSTOM_CHANNEL
    }

    override fun getView(context: Context) = DialogSettingsUpdateChannelBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChecker : SettingsItem.Toggle() {
    override val title = R.string.settings_check_update_title.asTransitive()
    override val description = R.string.settings_check_update_summary.asTransitive()
    override var value by bindableValue(Config.checkUpdate) {
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
    override var value by bindableValue(Config.suBiometric) { Config.suBiometric = it }
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
    override var value by bindableValue(Config.suReAuth) { Config.suReAuth = it }

    override fun refresh() {
        isEnabled = Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Utils.showSuperUser()
    }
}

// --- Magisk

object Magisk : SettingsItem.Section() {
    override val title = R.string.magisk.asTransitive()
}

object SafeMode : SettingsItem.Toggle() {
    override val title = R.string.settings_safe_mode_title.asTransitive()
    // Use old placeholder for now, will update text once native implementation is changed
    override val description = R.string.settings_core_only_summary.asTransitive()
    override var value by bindableValue(Config.coreOnly) {
        if (Config.coreOnly == it) return@bindableValue
        Config.coreOnly = it
        when {
            it -> runCatching { Const.MAGISK_DISABLE_FILE.createNewFile() }
            else -> Const.MAGISK_DISABLE_FILE.delete()
        }
        Utils.toast(R.string.settings_reboot_toast, Toast.LENGTH_LONG)
    }
}

object MagiskHide : SettingsItem.Toggle() {
    override val title = R.string.magiskhide.asTransitive()
    override val description = R.string.settings_magiskhide_summary.asTransitive()
    override var value by bindableValue(Config.magiskHide) {
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

    override var value by bindableValue(Config.rootMode) {
        Config.rootMode = entryValues[it].toInt()
    }
}

object MultiuserMode : SettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asTransitive()
    override val entryRes = R.array.multiuser_mode
    override val entryValRes = R.array.value_array

    override var value by bindableValue(Config.suMultiuserMode) {
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

    override var value by bindableValue(Config.suMntNamespaceMode) {
        Config.suMntNamespaceMode = entryValues[it].toInt()
    }

    override val description
        get() = resources.getStringArray(R.array.namespace_summary)[value].asTransitive()
}

object AutomaticResponse : SettingsItem.Selector() {
    override val title = R.string.auto_response.asTransitive()
    override val entryRes = R.array.auto_response
    override val entryValRes = R.array.value_array

    override var value by bindableValue(Config.suAutoReponse) {
        Config.suAutoReponse = entryValues[it].toInt()
    }
}

object RequestTimeout : SettingsItem.Selector() {
    override val title = R.string.request_timeout.asTransitive()
    override val entryRes = R.array.request_timeout
    override val entryValRes = R.array.request_timeout_value

    override var value by bindableValue(selected) {
        Config.suDefaultTimeout = entryValues[it].toInt()
    }

    private val selected: Int
        get() = entryValues.indexOfFirst { it.toInt() == Config.suDefaultTimeout }
}

object SUNotification : SettingsItem.Selector() {
    override val title = R.string.superuser_notification.asTransitive()
    override val entryRes = R.array.su_notification
    override val entryValRes = R.array.value_array

    override var value by bindableValue(Config.suNotification) {
        Config.suNotification = entryValues[it].toInt()
    }
}
