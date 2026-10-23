package com.topjohnwu.magisk.ui.settings

import androidx.compose.runtime.Immutable

sealed interface SettingsDialog {
    data class CustomChannelUrl(val initialUrl: String) : SettingsDialog
    data class DownloadPath(val initialPath: String) : SettingsDialog
}

@Immutable
data class SettingsUiState(
    // Environment & Section Visibility flags
    val hasRootSections: Boolean = false,
    val isMagiskInstalled: Boolean = false,
    val isSuperUserAvailable: Boolean = false,
    val isZygiskSupported: Boolean = false,
    val isSuRestrictSupported: Boolean = false,
    val isMultiuserSupported: Boolean = false,
    val isMultiuserEditable: Boolean = false,
    val isDeviceSecure: Boolean = false,
    val useLocaleManager: Boolean = false,

    // Customization Section
    val availableLanguages: List<String> = emptyList(),
    val availableLanguageTags: List<String> = emptyList(),
    val selectedLanguageIndex: Int = 0,
    val selectedColorModeIndex: Int = 0,
    val isDynamicColorSupported: Boolean = false,

    // App Settings Section
    val selectedUpdateChannelIndex: Int = 0,
    val isCustomChannel: Boolean = false,
    val customChannelUrl: String = "",
    val dohEnabled: Boolean = false,
    val checkUpdateEnabled: Boolean = true,
    val downloadPath: String = "",
    val downloadPathDisplay: String = "",
    val randomPackageNameEnabled: Boolean = true,

    // Magisk Section
    val zygiskEnabled: Boolean = false,
    val isZygiskPendingReboot: Boolean = false,
    val denyListEnabled: Boolean = false,

    // Superuser Section
    val suAuthEnabled: Boolean = false,
    val selectedMultiuserModeIndex: Int = 0,
    val selectedNamespaceModeIndex: Int = 0,
    val selectedNotificationModeIndex: Int = 0,
    val suRestrictEnabled: Boolean = false,

    // Active Modal Dialog
    val activeDialog: SettingsDialog? = null,
)
