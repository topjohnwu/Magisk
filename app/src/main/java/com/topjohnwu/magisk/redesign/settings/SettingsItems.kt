package com.topjohnwu.magisk.redesign.settings

import android.content.Context
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.utils.asTransitive

// --- Customization

object Customization : SettingsItem.Section() {
    override val title = "Customization".asTransitive()
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

object Language
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

object DownloadPath
object UpdateChannel
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

object AccessMode
object MultiuserMode
object MountNamespaceMode
object AutomaticResponse
object RequestTimeout
object SUNotification