package com.topjohnwu.magisk.ui.compose.settings

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.os.Build
import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.BrightnessMedium
import androidx.compose.material.icons.rounded.ChevronRight
import androidx.compose.material.icons.rounded.CloudDownload
import androidx.compose.material.icons.rounded.ColorLens
import androidx.compose.material.icons.rounded.Palette
import androidx.compose.material.icons.rounded.Security
import androidx.compose.material.icons.rounded.Shield
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.utils.LocaleSetting
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun SettingsScreen(
    onOpenDenyList: () -> Unit = {},
    viewModel: SettingsComposeViewModel = viewModel(factory = SettingsComposeViewModel.Factory)
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val activity = context as? UIActivity<*>
    val state = viewModel.state
    val snackbarHostState = remember { SnackbarHostState() }

    var selector by remember { mutableStateOf<SelectorSpec?>(null) }
    var input by remember { mutableStateOf<InputSpec?>(null) }
    var confirmRestore by remember { mutableStateOf(false) }

    LaunchedEffect(state.message) {
        val msg = state.message ?: return@LaunchedEffect
        snackbarHostState.showSnackbar(msg)
        viewModel.consumeMessage()
    }
    LaunchedEffect(Unit) {
        viewModel.refreshState()
    }
    DisposableEffect(lifecycleOwner) {
        val observer = LifecycleEventObserver { _, event ->
            if (event == Lifecycle.Event.ON_RESUME) {
                viewModel.refreshState()
            }
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose {
            lifecycleOwner.lifecycle.removeObserver(observer)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 120.dp, start = 24.dp, end = 24.dp, top = 8.dp),
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            item {
                OrganicSettingsSection(stringResource(id = CoreR.string.settings_section_personalization), Icons.Rounded.Palette) {
                    ExpressiveSettingItem(
                        title = stringResource(id = CoreR.string.settings_dark_mode_title),
                        subtitle = darkModeLabel(state.darkThemeMode),
                        icon = Icons.Rounded.BrightnessMedium,
                        onClick = {
                            selector = SelectorSpec(
                                title = AppContext.getString(CoreR.string.settings_dark_mode_title),
                                options = listOf(
                                    AppContext.getString(CoreR.string.settings_dark_mode_light),
                                    AppContext.getString(CoreR.string.settings_dark_mode_system),
                                    AppContext.getString(CoreR.string.settings_dark_mode_dark)
                                ),
                                selectedIndex = when (state.darkThemeMode) {
                                    AppCompatDelegate.MODE_NIGHT_NO -> 0
                                    AppCompatDelegate.MODE_NIGHT_YES -> 2
                                    else -> 1
                                },
                                onSelect = { idx ->
                                    val mode = when (idx) {
                                        0 -> AppCompatDelegate.MODE_NIGHT_NO
                                        2 -> AppCompatDelegate.MODE_NIGHT_YES
                                        else -> AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM
                                    }
                                    viewModel.setDarkMode(mode)
                                    activity?.delegate?.localNightMode = mode
                                }
                            )
                        }
                    )
                    ExpressiveSettingItem(
                        title = stringResource(id = CoreR.string.section_theme),
                        subtitle = state.themeName,
                        icon = Icons.Rounded.ColorLens,
                        onClick = {
                            selector = SelectorSpec(
                                title = AppContext.getString(CoreR.string.section_theme),
                                options = Theme.values().map { it.themeName },
                                selectedIndex = state.selectedThemeIndex,
                                onSelect = {
                                    viewModel.setThemeOrdinal(it)
                                    activity?.recreate()
                                }
                            )
                        }
                    )
                    if (state.useLocaleManager) {
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.language),
                            subtitle = state.languageSystemName,
                            onClick = {
                                runCatching {
                                    context.startActivity(LocaleSetting.localeSettingsIntent)
                                }
                            }
                        )
                    } else {
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.language),
                            subtitle = state.languageName,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.language),
                                    options = LocaleSetting.available.names.toList(),
                                    selectedIndex = state.languageIndex,
                                    onSelect = viewModel::setLanguageByIndex
                                )
                            }
                        )
                    }
                    if (state.canAddShortcut) {
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.add_shortcut_title),
                            subtitle = stringResource(id = CoreR.string.setting_add_shortcut_summary),
                            onClick = viewModel::addShortcut
                        )
                    }
                    if (state.canMigrateApp) {
                        if (state.isHiddenApp) {
                            ExpressiveSettingItem(
                                title = stringResource(id = CoreR.string.settings_restore_app_title),
                                subtitle = stringResource(id = CoreR.string.settings_restore_app_summary),
                                onClick = { confirmRestore = true }
                            )
                        } else {
                            ExpressiveSettingItem(
                                title = stringResource(id = CoreR.string.settings_hide_app_title),
                                subtitle = stringResource(id = CoreR.string.settings_hide_app_summary),
                                onClick = {
                                    input = InputSpec(
                                        title = AppContext.getString(CoreR.string.settings_hide_app_title),
                                        initialValue = AppContext.getString(CoreR.string.settings),
                                        onConfirm = { label -> viewModel.hideApp(activity, label) }
                                    )
                                }
                            )
                        }
                    }
                }
            }

            item {
                OrganicSettingsSection(stringResource(id = CoreR.string.settings_section_update_network), Icons.Rounded.CloudDownload) {
                    ExpressiveToggleItem(
                        title = stringResource(id = CoreR.string.settings_check_update_title),
                        subtitle = stringResource(id = CoreR.string.settings_check_update_summary),
                        checked = state.checkUpdate,
                        onChecked = { checked ->
                            activity?.withPermission(POST_NOTIFICATIONS) { granted ->
                                if (granted) {
                                    viewModel.setCheckUpdate(checked)
                                } else {
                                    viewModel.setMessageRes(CoreR.string.post_notifications_denied)
                                }
                            } ?: viewModel.setMessageRes(CoreR.string.app_not_found)
                        }
                    )
                    ExpressiveSettingItem(
                        title = stringResource(id = CoreR.string.settings_update_channel_title),
                        subtitle = state.updateChannelName,
                        onClick = {
                            selector = SelectorSpec(
                                title = AppContext.getString(CoreR.string.settings_update_channel_title),
                                options = context.resources.getStringArray(CoreR.array.update_channel).toList(),
                                selectedIndex = state.updateChannel.coerceAtLeast(0),
                                onSelect = { index ->
                                    viewModel.setUpdateChannel(index)
                                    if (index == Config.Value.CUSTOM_CHANNEL && viewModel.state.customChannelUrl.isBlank()) {
                                        input = InputSpec(
                                            title = AppContext.getString(CoreR.string.settings_update_custom),
                                            initialValue = "",
                                            onConfirm = viewModel::setCustomChannelUrl
                                        )
                                    }
                                }
                            )
                        }
                    )
                    ExpressiveSettingItem(
                        title = stringResource(id = CoreR.string.settings_update_custom),
                        subtitle = state.customChannelUrl.ifBlank { stringResource(id = CoreR.string.settings_update_custom_msg) },
                        enabled = state.isCustomChannel,
                        onClick = {
                            input = InputSpec(
                                title = AppContext.getString(CoreR.string.settings_update_custom),
                                initialValue = state.customChannelUrl,
                                onConfirm = viewModel::setCustomChannelUrl
                            )
                        }
                    )
                    ExpressiveToggleItem(
                        title = stringResource(id = CoreR.string.settings_doh_title),
                        subtitle = stringResource(id = CoreR.string.settings_doh_description),
                        checked = state.doh,
                        onChecked = viewModel::setDoH
                    )
                    ExpressiveSettingItem(
                        title = stringResource(id = CoreR.string.settings_download_path_title),
                        subtitle = state.downloadDirPath.ifBlank { "-" },
                        onClick = {
                            activity?.withPermission(WRITE_EXTERNAL_STORAGE) { granted ->
                                if (granted) {
                                    input = InputSpec(
                                        title = AppContext.getString(CoreR.string.settings_download_path_title),
                                        initialValue = state.downloadDir,
                                        onConfirm = viewModel::setDownloadDir
                                    )
                                } else {
                                    viewModel.setMessageRes(CoreR.string.external_rw_permission_denied)
                                }
                            } ?: viewModel.setMessageRes(CoreR.string.app_not_found)
                        }
                    )
                    ExpressiveToggleItem(
                        title = stringResource(id = CoreR.string.settings_random_name_title),
                        subtitle = stringResource(id = CoreR.string.settings_random_name_description),
                        checked = state.randName,
                        onChecked = viewModel::setRandName
                    )
                }
            }
            if (state.showMagisk) {
                item {
                    OrganicSettingsSection(stringResource(id = CoreR.string.home_section_magisk_core), Icons.Rounded.Security) {
                        if (state.showMagiskAdvanced) {
                            ExpressiveToggleItem(
                                title = stringResource(id = CoreR.string.zygisk),
                                subtitle = if (state.zygiskMismatch) {
                                    stringResource(id = CoreR.string.reboot_apply_change)
                                } else {
                                    stringResource(id = CoreR.string.settings_zygisk_summary)
                                },
                                checked = state.zygisk,
                                onChecked = viewModel::setZygisk
                            )
                            ExpressiveToggleItem(
                                title = stringResource(id = CoreR.string.settings_denylist_title),
                                subtitle = stringResource(id = CoreR.string.settings_denylist_summary),
                                checked = state.denyList,
                                onChecked = viewModel::setDenyList
                            )
                            if (state.showDenyListConfig) {
                                ExpressiveSettingItem(
                                    title = stringResource(id = CoreR.string.settings_denylist_config_title),
                                    subtitle = stringResource(id = CoreR.string.settings_denylist_config_summary),
                                    onClick = onOpenDenyList
                                )
                            }
                        }
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.settings_hosts_title),
                            subtitle = stringResource(id = CoreR.string.settings_hosts_summary),
                            onClick = viewModel::createSystemlessHosts
                        )
                    }
                }
            }

            if (state.showSuperuser) {
                item {
                    OrganicSettingsSection(stringResource(id = CoreR.string.superuser), Icons.Rounded.Shield) {
                        if (!state.hideTapjackOnSPlus) {
                            ExpressiveToggleItem(
                                title = stringResource(id = CoreR.string.settings_su_tapjack_title),
                                subtitle = stringResource(id = CoreR.string.settings_su_tapjack_summary),
                                checked = state.suTapjack,
                                onChecked = viewModel::setSuTapjack
                            )
                        }
                        ExpressiveToggleItem(
                            title = stringResource(id = CoreR.string.settings_su_auth_title),
                            subtitle = if (state.deviceSecure) {
                                stringResource(id = CoreR.string.settings_su_auth_summary)
                            } else {
                                stringResource(id = CoreR.string.settings_su_auth_insecure)
                            },
                            checked = state.suAuth,
                            enabled = state.deviceSecure,
                            onChecked = { checked ->
                                activity?.withAuthentication { ok ->
                                    if (ok) viewModel.setSuAuth(checked)
                                } ?: viewModel.setMessageRes(CoreR.string.app_not_found)
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.superuser_access),
                            subtitle = state.accessModeName,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.superuser_access),
                                    options = context.resources.getStringArray(CoreR.array.su_access).toList(),
                                    selectedIndex = state.rootMode.coerceAtLeast(0),
                                    onSelect = viewModel::setRootMode
                                )
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.multiuser_mode),
                            subtitle = state.multiuserSummary,
                            enabled = state.multiuserModeEnabled,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.multiuser_mode),
                                    options = context.resources.getStringArray(CoreR.array.multiuser_mode).toList(),
                                    selectedIndex = state.suMultiuserMode.coerceAtLeast(0),
                                    onSelect = viewModel::setSuMultiuserMode
                                )
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.mount_namespace_mode),
                            subtitle = state.mountNamespaceSummary,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.mount_namespace_mode),
                                    options = context.resources.getStringArray(CoreR.array.namespace).toList(),
                                    selectedIndex = state.suMntNamespaceMode.coerceAtLeast(0),
                                    onSelect = viewModel::setSuMntNamespaceMode
                                )
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.auto_response),
                            subtitle = state.autoResponseName,
                            onClick = {
                                val showSelector = {
                                    selector = SelectorSpec(
                                        title = AppContext.getString(CoreR.string.auto_response),
                                        options = context.resources.getStringArray(CoreR.array.auto_response).toList(),
                                        selectedIndex = state.suAutoResponse.coerceAtLeast(0),
                                        onSelect = viewModel::setSuAutoResponse
                                    )
                                }
                                if (state.suAuth) {
                                    activity?.withAuthentication { ok -> if (ok) showSelector() }
                                        ?: viewModel.setMessageRes(CoreR.string.app_not_found)
                                } else {
                                    showSelector()
                                }
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.request_timeout),
                            subtitle = state.requestTimeoutName,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.request_timeout),
                                    options = context.resources.getStringArray(CoreR.array.request_timeout).toList(),
                                    selectedIndex = state.suTimeoutIndex.coerceIn(0, SU_TIMEOUT_VALUES.lastIndex),
                                    onSelect = viewModel::setSuTimeoutIndex
                                )
                            }
                        )
                        ExpressiveSettingItem(
                            title = stringResource(id = CoreR.string.superuser_notification),
                            subtitle = state.suNotificationName,
                            onClick = {
                                selector = SelectorSpec(
                                    title = AppContext.getString(CoreR.string.superuser_notification),
                                    options = context.resources.getStringArray(CoreR.array.su_notification).toList(),
                                    selectedIndex = state.suNotification.coerceAtLeast(0),
                                    onSelect = viewModel::setSuNotification
                                )
                            }
                        )
                        if (state.showReauthenticate) {
                            ExpressiveToggleItem(
                                title = stringResource(id = CoreR.string.settings_su_reauth_title),
                                subtitle = stringResource(id = CoreR.string.settings_su_reauth_summary),
                                checked = state.suReAuth,
                                onChecked = viewModel::setSuReAuth
                            )
                        }
                        if (state.showRestrict) {
                            ExpressiveToggleItem(
                                title = stringResource(id = CoreR.string.settings_su_restrict_title),
                                subtitle = stringResource(id = CoreR.string.settings_su_restrict_summary),
                                checked = state.suRestrict,
                                onChecked = viewModel::setSuRestrict
                            )
                        }
                    }
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(bottom = 110.dp)
        )
    }

    selector?.let { spec ->
        AlertDialog(
            onDismissRequest = { selector = null },
            title = { Text(spec.title, fontWeight = FontWeight.Black) },
            text = {
                Column {
                    spec.options.forEachIndexed { index, label ->
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .clip(RoundedCornerShape(16.dp))
                                .clickable {
                                    spec.onSelect(index)
                                    selector = null
                                }
                                .padding(16.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(selected = index == spec.selectedIndex, onClick = null)
                            Spacer(Modifier.width(16.dp))
                            Text(label, style = MaterialTheme.typography.bodyLarge)
                        }
                    }
                }
            },
            confirmButton = {}
        )
    }

    input?.let { spec ->
        var value by remember(spec.initialValue) { mutableStateOf(spec.initialValue) }
        AlertDialog(
            onDismissRequest = { input = null },
            title = { Text(spec.title, fontWeight = FontWeight.Black) },
            text = {
                OutlinedTextField(
                    value = value,
                    onValueChange = { value = it },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth(),
                    shape = RoundedCornerShape(16.dp)
                )
            },
            confirmButton = {
                TextButton(onClick = {
                    spec.onConfirm(value.trim())
                    input = null
                }) {
                    Text(stringResource(id = android.R.string.ok), fontWeight = FontWeight.Bold)
                }
            }
        )
    }

    if (confirmRestore) {
        AlertDialog(
            onDismissRequest = { confirmRestore = false },
            title = { Text(stringResource(id = CoreR.string.settings_restore_app_title), fontWeight = FontWeight.Black) },
            text = { Text(stringResource(id = CoreR.string.restore_app_confirmation)) },
            confirmButton = {
                TextButton(onClick = {
                    confirmRestore = false
                    viewModel.restoreApp(activity)
                }) {
                    Text(stringResource(id = android.R.string.ok), fontWeight = FontWeight.Bold)
                }
            },
            dismissButton = {
                TextButton(onClick = { confirmRestore = false }) {
                    Text(stringResource(id = android.R.string.cancel))
                }
            }
        )
    }
}

@Composable
private fun OrganicSettingsSection(
    title: String,
    icon: ImageVector,
    content: @Composable ColumnScope.() -> Unit
) {
    Column {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.padding(start = 8.dp, bottom = 12.dp)
        ) {
            Icon(icon, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.primary)
            Spacer(Modifier.width(12.dp))
            Text(
                text = title.uppercase(),
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Black,
                letterSpacing = 1.5.sp,
                color = MaterialTheme.colorScheme.outline
            )
        }
        ElevatedCard(
            shape = RoundedCornerShape(topEnd = 40.dp, bottomStart = 40.dp, topStart = 12.dp, bottomEnd = 12.dp),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
        ) {
            Column(modifier = Modifier.padding(vertical = 12.dp)) {
                content()
            }
        }
    }
}
@Composable
private fun ExpressiveSettingItem(
    title: String,
    subtitle: String,
    icon: ImageVector? = null,
    enabled: Boolean = true,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(enabled = enabled, onClick = onClick)
            .padding(20.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        if (icon != null) {
            Surface(
                color = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f),
                shape = CircleShape,
                modifier = Modifier.size(40.dp)
            ) {
                Icon(icon, null, modifier = Modifier.padding(10.dp), tint = MaterialTheme.colorScheme.primary)
            }
            Spacer(Modifier.width(20.dp))
        }
        Column(Modifier.weight(1f)) {
            Text(
                title,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = if (enabled) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.outline
            )
            Text(
                subtitle,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        Icon(
            Icons.Rounded.ChevronRight,
            null,
            tint = if (enabled) MaterialTheme.colorScheme.outline else MaterialTheme.colorScheme.outline.copy(alpha = 0.4f)
        )
    }
}

@Composable
private fun ExpressiveToggleItem(
    title: String,
    subtitle: String,
    checked: Boolean,
    enabled: Boolean = true,
    onChecked: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(enabled = enabled) { onChecked(!checked) }
            .padding(20.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(Modifier.weight(1f)) {
            Text(
                title,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = if (enabled) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.outline
            )
            Text(
                subtitle,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                lineHeight = 18.sp
            )
        }
        Spacer(Modifier.width(16.dp))
        Switch(checked = checked, onCheckedChange = onChecked, enabled = enabled)
    }
}

private fun darkModeLabel(mode: Int): String = when (mode) {
    AppCompatDelegate.MODE_NIGHT_NO -> AppContext.getString(CoreR.string.settings_dark_mode_light)
    AppCompatDelegate.MODE_NIGHT_YES -> AppContext.getString(CoreR.string.settings_dark_mode_dark)
    else -> AppContext.getString(CoreR.string.settings_dark_mode_system)
}

data class SelectorSpec(
    val title: String,
    val options: List<String>,
    val selectedIndex: Int,
    val onSelect: (Int) -> Unit
)

data class InputSpec(
    val title: String,
    val initialValue: String,
    val onConfirm: (String) -> Unit
)

data class SettingsUiState(
    val darkThemeMode: Int = Config.darkTheme,
    val themeOrdinal: Int = Config.themeOrdinal,
    val selectedThemeIndex: Int = Theme.values().indexOf(Theme.selected).coerceAtLeast(0),
    val themeName: String = Theme.selected.themeName,
    val useLocaleManager: Boolean = LocaleSetting.useLocaleManager,
    val languageSystemName: String = LocaleSetting.instance.appLocale?.let {
        it.getDisplayName(it)
    } ?: AppContext.getString(CoreR.string.system_default),
    val languageIndex: Int = LocaleSetting.available.tags.indexOf(Config.locale).let { if (it < 0) 0 else it },
    val languageName: String = LocaleSetting.available.names.getOrElse(
        LocaleSetting.available.tags.indexOf(Config.locale).let { if (it < 0) 0 else it }
    ) { AppContext.getString(CoreR.string.system_default) },
    val canAddShortcut: Boolean = isRunningAsStub && ShortcutManagerCompat.isRequestPinShortcutSupported(AppContext),
    val canMigrateApp: Boolean = Info.env.isActive && Const.USER_ID == 0,
    val isHiddenApp: Boolean = AppContext.packageName != BuildConfig.APP_PACKAGE_NAME,
    val checkUpdate: Boolean = Config.checkUpdate,
    val updateChannel: Int = Config.updateChannel,
    val isCustomChannel: Boolean = Config.updateChannel == Config.Value.CUSTOM_CHANNEL,
    val updateChannelName: String = AppContext.resources.getStringArray(CoreR.array.update_channel).getOrElse(Config.updateChannel) { "-" },
    val customChannelUrl: String = Config.customChannelUrl,
    val doh: Boolean = Config.doh,
    val downloadDir: String = Config.downloadDir,
    val downloadDirPath: String = MediaStoreUtils.fullPath(Config.downloadDir),
    val randName: Boolean = Config.randName,
    val zygisk: Boolean = Config.zygisk,
    val zygiskMismatch: Boolean = Config.zygisk != Info.isZygiskEnabled,
    val denyList: Boolean = Config.denyList,
    val showMagisk: Boolean = Info.env.isActive,
    val showMagiskAdvanced: Boolean = Info.env.isActive && Const.Version.atLeast_24_0(),
    val showDenyListConfig: Boolean = Const.Version.atLeast_24_0(),
    val showSuperuser: Boolean = Info.showSuperUser,
    val deviceSecure: Boolean = Info.isDeviceSecure,
    val suTapjack: Boolean = Config.suTapjack,
    val suAuth: Boolean = Config.suAuth,
    val hideTapjackOnSPlus: Boolean = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S,
    val rootMode: Int = Config.rootMode,
    val accessModeName: String = AppContext.resources.getStringArray(CoreR.array.su_access).getOrElse(Config.rootMode) { "-" },
    val suMultiuserMode: Int = Config.suMultiuserMode,
    val multiuserModeName: String = AppContext.resources.getStringArray(CoreR.array.multiuser_mode).getOrElse(Config.suMultiuserMode) { "-" },
    val multiuserModeEnabled: Boolean = Const.USER_ID == 0,
    val multiuserSummary: String = AppContext.resources.getStringArray(CoreR.array.multiuser_summary).getOrElse(Config.suMultiuserMode) { "-" },
    val suMntNamespaceMode: Int = Config.suMntNamespaceMode,
    val mountNamespaceName: String = AppContext.resources.getStringArray(CoreR.array.namespace).getOrElse(Config.suMntNamespaceMode) { "-" },
    val mountNamespaceSummary: String = AppContext.resources.getStringArray(CoreR.array.namespace_summary).getOrElse(Config.suMntNamespaceMode) { "-" },
    val suAutoResponse: Int = Config.suAutoResponse,
    val autoResponseName: String = AppContext.resources.getStringArray(CoreR.array.auto_response).getOrElse(Config.suAutoResponse) { "-" },
    val suTimeoutIndex: Int = SU_TIMEOUT_VALUES.indexOf(Config.suDefaultTimeout).let { if (it < 0) 0 else it },
    val requestTimeoutName: String = AppContext.resources.getStringArray(CoreR.array.request_timeout).getOrElse(
        SU_TIMEOUT_VALUES.indexOf(Config.suDefaultTimeout).let { if (it < 0) 0 else it }
    ) { "-" },
    val suNotification: Int = Config.suNotification,
    val suNotificationName: String = AppContext.resources.getStringArray(CoreR.array.su_notification).getOrElse(Config.suNotification) { "-" },
    val suReAuth: Boolean = Config.suReAuth,
    val showReauthenticate: Boolean = Build.VERSION.SDK_INT < Build.VERSION_CODES.O,
    val suRestrict: Boolean = Config.suRestrict,
    val showRestrict: Boolean = Const.Version.atLeast_30_1(),
    val message: String? = null
)

class SettingsComposeViewModel : ViewModel() {
    var state by mutableStateOf(snapshotState())
        private set

    fun refreshState() {
        state = snapshotState(state.message)
    }

    fun setDarkMode(mode: Int) {
        Config.darkTheme = mode
        state = snapshotState()
    }

    fun setThemeOrdinal(index: Int) {
        val theme = Theme.values().getOrNull(index) ?: Theme.Default
        Config.themeOrdinal = if (theme == Theme.Default) -1 else theme.ordinal
        state = snapshotState()
    }

    fun setLanguageByIndex(index: Int) {
        if (state.useLocaleManager) return
        val tags = LocaleSetting.available.tags
        if (tags.isEmpty()) return
        val safe = index.coerceIn(0, tags.lastIndex)
        Config.locale = tags[safe]
        state = snapshotState()
    }

    fun addShortcut() {
        runCatching {
            Shortcuts.addHomeIcon(AppContext)
            state = snapshotState()
        }.onFailure {
            state = snapshotState(AppContext.getString(CoreR.string.failure))
        }
    }

    fun hideApp(activity: UIActivity<*>?, label: String) {
        val safeLabel = label.trim()
        if (activity == null || safeLabel.isBlank() || safeLabel.length > AppMigration.MAX_LABEL_LENGTH) {
            state = snapshotState(AppContext.getString(CoreR.string.failure))
            return
        }
        viewModelScope.launch {
            AppMigration.hide(activity, safeLabel)
            state = snapshotState()
        }
    }

    fun restoreApp(activity: UIActivity<*>?) {
        if (activity == null) {
            state = snapshotState(AppContext.getString(CoreR.string.failure))
            return
        }
        viewModelScope.launch {
            AppMigration.restore(activity)
            state = snapshotState()
        }
    }

    fun setCheckUpdate(v: Boolean) {
        Config.checkUpdate = v
        state = snapshotState()
    }

    fun setUpdateChannel(c: Int) {
        Config.updateChannel = c
        Info.resetUpdate()
        state = snapshotState()
    }

    fun setCustomChannelUrl(u: String) {
        Config.customChannelUrl = u
        Info.resetUpdate()
        state = snapshotState()
    }

    fun setDoH(v: Boolean) {
        Config.doh = v
        state = snapshotState()
    }

    fun setDownloadDir(v: String) {
        Config.downloadDir = v
        state = snapshotState()
    }

    fun setRandName(v: Boolean) {
        Config.randName = v
        state = snapshotState()
    }

    fun createSystemlessHosts() {
        viewModelScope.launch {
            val ok = RootUtils.addSystemlessHosts()
            state = snapshotState(
                AppContext.getString(if (ok) CoreR.string.settings_hosts_toast else CoreR.string.failure)
            )
        }
    }

    fun setZygisk(v: Boolean) {
        Config.zygisk = v
        val mismatch = v != Info.isZygiskEnabled
        state = if (mismatch) {
            snapshotState(AppContext.getString(CoreR.string.reboot_apply_change))
        } else {
            snapshotState()
        }
    }

    fun setDenyList(v: Boolean) {
        viewModelScope.launch {
            val cmd = if (v) "enable" else "disable"
            val ok = withContext(Dispatchers.IO) {
                Shell.cmd("magisk --denylist $cmd").exec().isSuccess
            }
            state = if (ok) {
                Config.denyList = v
                snapshotState()
            } else {
                snapshotState(AppContext.getString(CoreR.string.failure))
            }
        }
    }

    fun setRootMode(v: Int) {
        Config.rootMode = v
        state = snapshotState()
    }

    fun setSuMultiuserMode(v: Int) {
        Config.suMultiuserMode = v
        state = snapshotState()
    }

    fun setSuMntNamespaceMode(v: Int) {
        Config.suMntNamespaceMode = v
        state = snapshotState()
    }

    fun setSuAuth(v: Boolean) {
        Config.suAuth = v
        state = snapshotState()
    }

    fun setSuAutoResponse(v: Int) {
        Config.suAutoResponse = v
        state = snapshotState()
    }

    fun setSuTimeoutIndex(index: Int) {
        val safe = index.coerceIn(0, SU_TIMEOUT_VALUES.lastIndex)
        Config.suDefaultTimeout = SU_TIMEOUT_VALUES[safe]
        state = snapshotState()
    }

    fun setSuNotification(v: Int) {
        Config.suNotification = v
        state = snapshotState()
    }

    fun setSuReAuth(v: Boolean) {
        Config.suReAuth = v
        state = snapshotState()
    }

    fun setSuTapjack(v: Boolean) {
        Config.suTapjack = v
        state = snapshotState()
    }

    fun setSuRestrict(v: Boolean) {
        Config.suRestrict = v
        state = snapshotState()
    }

    fun consumeMessage() {
        state = snapshotState()
    }

    fun setMessageRes(res: Int) {
        state = snapshotState(AppContext.getString(res))
    }

    private fun snapshotState(message: String? = null): SettingsUiState {
        return SettingsUiState(message = message)
    }

    companion object {
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return SettingsComposeViewModel() as T
            }
        }
    }
}

private val SU_TIMEOUT_VALUES = Config.Value.TIMEOUT_LIST.map { it.toInt() }
