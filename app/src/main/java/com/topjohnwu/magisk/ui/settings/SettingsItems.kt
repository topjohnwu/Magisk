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
    override val title = "Customization".asTransitive()
}

object Language : SettingsItem.Selector() {
    override var value by bindableValue(0) {
        Config.locale = entryValues.getOrNull(it)?.toString() ?: return@bindableValue
        refreshLocale()
    }

    override val title = R.string.language.asTransitive()

    init {
        availableLocales.subscribeK { (names, values) ->
            setValues(names, values)
            val selectedLocale = currentLocale.getDisplayName(
                currentLocale
            )
            value = names.indexOfFirst { it == selectedLocale }.let { if (it == -1) 0 else it }
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
    val isError
        @Bindable get() = value.length > 14 || value.isBlank()

    override val intermediate: String?
        get() = if (isError) null else value

    override fun getView(context: Context) = DialogSettingsAppNameBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
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
    var result = value
        @Bindable get
        set(value) {
            field = value
            notifyChange(BR.result)
            notifyChange(BR.path)
        }
    val path
        @Bindable get() = File(
            Environment.getExternalStorageDirectory(),
            result
        ).absolutePath.orEmpty()

    override fun getView(context: Context) = DialogSettingsDownloadPathBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object GridSize : SettingsItem.Selector() {
    override var value by bindableValue(Config.listSpanCount - 1) {
        Config.listSpanCount = max(1, min(3, it + 1))
    }

    override val title = R.string.settings_grid_span_count_title.asTransitive()
    override val description = R.string.settings_grid_span_count_summary.asTransitive()

    init {
        setValues(
            resources.getStringArray(R.array.span_count),
            resources.getStringArray(R.array.value_array)
        )
    }
}

object UpdateChannel : SettingsItem.Selector() {
    override var value by bindableValue(Config.updateChannel) { Config.updateChannel = it }
    override val title = R.string.settings_update_channel_title.asTransitive()

    init {
        val entries = resources.getStringArray(R.array.update_channel).let {
            if (!Utils.isCanary && Config.updateChannel < Config.Value.CANARY_CHANNEL)
                it.take(it.size - 2).toTypedArray() else it
        }
        setValues(
            entries,
            resources.getStringArray(R.array.value_array)
        )
    }
}

object UpdateChannelUrl : SettingsItem.Input() {
    override val title = R.string.settings_update_custom.asTransitive()
    override var value: String by bindableValue(Config.customChannelUrl) {
        Config.customChannelUrl = it
    }
    override val intermediate: String? get() = result

    var result = value
        @Bindable get
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

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
}

object Biometrics : SettingsItem.Toggle() {
    override val title = R.string.settings_su_biometric_title.asTransitive()
    override var value by bindableValue(Config.suBiometric) { Config.suBiometric = it }
    override var description = R.string.settings_su_biometric_summary.asTransitive()

    override fun refresh() {
        isEnabled = BiometricHelper.isSupported && Utils.showSuperUser()
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

    override fun refresh() {
        isEnabled = Info.env.isActive
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

    override fun refresh() {
        isEnabled = Info.env.isActive
    }
}

// --- Superuser

object Superuser : SettingsItem.Section() {
    override val title = R.string.superuser.asTransitive()
}

object AccessMode : SettingsItem.Selector() {
    override val title = R.string.superuser_access.asTransitive()
    override var value by bindableValue(Config.rootMode) {
        Config.rootMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }

    init {
        setValues(
            resources.getStringArray(R.array.su_access),
            resources.getStringArray(R.array.value_array)
        )
    }

    override fun refresh() {
        isEnabled = Utils.showSuperUser()
    }
}

object MultiuserMode : SettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asTransitive()
    override var value by bindableValue(Config.suMultiuserMode) {
        Config.suMultiuserMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }
    private val descArray = resources.getStringArray(R.array.multiuser_summary)
    override val description
        get() = descArray[value].asTransitive()

    init {
        setValues(
            resources.getStringArray(R.array.multiuser_mode),
            resources.getStringArray(R.array.value_array)
        )
    }

    override fun refresh() {
        isEnabled = Const.USER_ID <= 0 && Utils.showSuperUser()
    }
}

object MountNamespaceMode : SettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asTransitive()
    override var value by bindableValue(Config.suMntNamespaceMode) {
        Config.suMntNamespaceMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }
    private val descArray = resources.getStringArray(R.array.namespace_summary)
    override val description
        get() = descArray[value].asTransitive()

    init {
        setValues(
            resources.getStringArray(R.array.namespace),
            resources.getStringArray(R.array.value_array)
        )
    }

    override fun refresh() {
        isEnabled = Utils.showSuperUser()
    }
}

object AutomaticResponse : SettingsItem.Selector() {
    override val title = R.string.auto_response.asTransitive()
    override var value by bindableValue(Config.suAutoReponse) {
        Config.suAutoReponse = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }

    init {
        setValues(
            resources.getStringArray(R.array.auto_response),
            resources.getStringArray(R.array.value_array)
        )
    }

    override fun refresh() {
        isEnabled = Utils.showSuperUser()
    }
}

object RequestTimeout : SettingsItem.Selector() {
    override val title = R.string.request_timeout.asTransitive()
    override var value by bindableValue(-1) {
        Config.suDefaultTimeout = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }

    init {
        setValues(
            resources.getStringArray(R.array.request_timeout),
            resources.getStringArray(R.array.request_timeout_value)
        )
        val currentValue = Config.suDefaultTimeout.toString()
        value = entryValues.indexOfFirst { it == currentValue }
    }

    override fun refresh() {
        isEnabled = Utils.showSuperUser()
    }
}

object SUNotification : SettingsItem.Selector() {
    override val title = R.string.superuser_notification.asTransitive()
    override var value by bindableValue(Config.suNotification) {
        Config.suNotification = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@bindableValue
    }

    init {
        setValues(
            resources.getStringArray(R.array.su_notification),
            resources.getStringArray(R.array.value_array)
        )
    }

    override fun refresh() {
        isEnabled = Utils.showSuperUser()
    }
}
