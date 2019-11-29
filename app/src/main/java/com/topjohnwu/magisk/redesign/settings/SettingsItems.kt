package com.topjohnwu.magisk.redesign.settings

import android.content.Context
import android.os.Environment
import android.view.LayoutInflater
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.DialogSettingsDownloadPathBinding
import com.topjohnwu.magisk.databinding.DialogSettingsUpdateChannelBinding
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.asTransitive
import com.topjohnwu.magisk.utils.availableLocales
import com.topjohnwu.magisk.utils.currentLocale
import java.io.File

// --- Customization

object Customization : SettingsItem.Section() {
    override val title = "Customization".asTransitive()
}

object Language : SettingsItem.Selector() {
    override var value by dataObservable(0) {
        Config.locale = entryValues.getOrNull(it)?.toString() ?: return@dataObservable
    }

    override val title = R.string.language.asTransitive()

    init {
        availableLocales.subscribeK { (names, values) ->
            setValues(names, values)
            val selectedLocale = currentLocale.getDisplayName(currentLocale)
            value = names.indexOfFirst { it == selectedLocale }.let { if (it == -1) 0 else it }
        }
    }
}

object Redesign : SettingsItem.Toggle() {
    override val title = "Redesign".asTransitive()
    override val description =
        "Select this to disable redesign. App will automatically shut down".asTransitive()
    override var value: Boolean by dataObservable(Config.redesign) { Config.redesign = it }
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
}

object Hide : SettingsItem.Blank() {
    override val title = R.string.settings_hide_manager_title.asTransitive()
    override val description = R.string.settings_hide_manager_summary.asTransitive()
}

object Restore : SettingsItem.Blank() {
    override val title = R.string.settings_restore_manager_title.asTransitive()
    override val description = R.string.settings_restore_manager_summary.asTransitive()
}

@Suppress("FunctionName")
fun HideOrRestore() =
    if (get<Context>().packageName == BuildConfig.APPLICATION_ID) Hide else Restore

object DownloadPath : SettingsItem.Input() {
    override var value: String by dataObservable(Config.downloadPath) { Config.downloadPath = it }
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

object UpdateChannel : SettingsItem.Selector() {
    override var value by dataObservable(Config.updateChannel) { Config.updateChannel = it }
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
    override var value: String by dataObservable(Config.customChannelUrl) {
        Config.customChannelUrl = it
    }
    override val intermediate: String? get() = result

    var result = value
        @Bindable get
        set(value) {
            field = value
            notifyChange(BR.result)
        }

    init {
        updateState()
    }

    fun updateState() {
        isEnabled = UpdateChannel.value == Config.Value.CUSTOM_CHANNEL
    }

    override fun getView(context: Context) = DialogSettingsUpdateChannelBinding
        .inflate(LayoutInflater.from(context)).also { it.data = this }.root
}

object UpdateChecker : SettingsItem.Toggle() {
    override val title = R.string.settings_check_update_title.asTransitive()
    override val description = R.string.settings_check_update_summary.asTransitive()
    override var value by dataObservable(Config.checkUpdate) { Config.checkUpdate = it }
}

// check whether is module already installed beforehand?
object SystemlessHosts : SettingsItem.Blank() {
    override val title = R.string.settings_hosts_title.asTransitive()
    override val description = R.string.settings_hosts_summary.asTransitive()
}

object Biometrics : SettingsItem.Toggle() {
    override val title = R.string.settings_su_biometric_title.asTransitive()
    override val description = R.string.settings_su_biometric_summary.asTransitive()
    override var value by dataObservable(Config.suBiometric) { Config.suBiometric = it }
}

// --- Magisk

object Magisk : SettingsItem.Section() {
    override val title = R.string.magisk.asTransitive()
}

object SafeMode : SettingsItem.Toggle() {
    override val title = R.string.settings_core_only_title.asTransitive()
    override val description = R.string.settings_core_only_summary.asTransitive()
    override var value by dataObservable(Config.coreOnly) { Config.coreOnly = it }
}

object MagiskHide : SettingsItem.Toggle() {
    override val title = R.string.magiskhide.asTransitive()
    override val description = R.string.settings_magiskhide_summary.asTransitive()
    override var value by dataObservable(Config.magiskHide) { Config.magiskHide = it }
}

// --- Superuser

object Superuser : SettingsItem.Section() {
    override val title = R.string.superuser.asTransitive()
}

object AccessMode : SettingsItem.Selector() {
    override val title = R.string.superuser_access.asTransitive()
    override var value by dataObservable(Config.rootMode) {
        Config.rootMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.su_access),
            resources.getStringArray(R.array.value_array)
        )
    }
}

object MultiuserMode : SettingsItem.Selector() {
    override val title = R.string.multiuser_mode.asTransitive()
    override var value by dataObservable(Config.suMultiuserMode) {
        Config.suMultiuserMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.multiuser_mode),
            resources.getStringArray(R.array.value_array)
        )
    }
}

object MountNamespaceMode : SettingsItem.Selector() {
    override val title = R.string.mount_namespace_mode.asTransitive()
    override var value by dataObservable(Config.suMntNamespaceMode) {
        Config.suMntNamespaceMode = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.namespace),
            resources.getStringArray(R.array.value_array)
        )
    }
}

object AutomaticResponse : SettingsItem.Selector() {
    override val title = R.string.auto_response.asTransitive()
    override var value by dataObservable(Config.suAutoReponse) {
        Config.suAutoReponse = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.auto_response),
            resources.getStringArray(R.array.value_array)
        )
    }
}

object RequestTimeout : SettingsItem.Selector() {
    override val title = R.string.request_timeout.asTransitive()
    override var value by dataObservable(-1) {
        Config.suDefaultTimeout = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.request_timeout),
            resources.getStringArray(R.array.request_timeout_value)
        )
        val currentValue = Config.suDefaultTimeout.toString()
        value = entryValues.indexOfFirst { it == currentValue }
    }
}

object SUNotification : SettingsItem.Selector() {
    override val title = R.string.superuser_notification.asTransitive()
    override var value by dataObservable(Config.suNotification) {
        Config.suNotification = entryValues.getOrNull(it)
            ?.toString()
            ?.toInt() ?: return@dataObservable
    }

    init {
        setValues(
            resources.getStringArray(R.array.su_notification),
            resources.getStringArray(R.array.value_array)
        )
    }
}