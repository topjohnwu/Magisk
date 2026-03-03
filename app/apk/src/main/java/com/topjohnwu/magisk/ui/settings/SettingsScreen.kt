package com.topjohnwu.magisk.ui.settings

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.unit.DpSize
import androidx.compose.ui.unit.dp
import androidx.core.content.pm.ShortcutManagerCompat
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.utils.LocaleSetting
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import top.yukonga.miuix.kmp.basic.ButtonDefaults
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.SmallTitle
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.basic.TextField
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.extra.SuperArrow
import top.yukonga.miuix.kmp.extra.SuperDropdown
import top.yukonga.miuix.kmp.extra.SuperSwitch
import top.yukonga.miuix.kmp.extra.SuperDialog
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun SettingsScreen(viewModel: SettingsViewModel) {
    val scrollBehavior = MiuixScrollBehavior()
    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.settings),
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .verticalScroll(rememberScrollState())
                .padding(padding)
                .padding(horizontal = 12.dp)
                .padding(bottom = 88.dp)
        ) {
            CustomizationSection(viewModel)
            Spacer(Modifier.height(12.dp))
            AppSettingsSection(viewModel)
            if (Info.env.isActive) {
                Spacer(Modifier.height(12.dp))
                MagiskSection(viewModel)
            }
            if (Info.showSuperUser) {
                Spacer(Modifier.height(12.dp))
                SuperuserSection(viewModel)
            }
        }
    }
}

// --- Customization ---

@Composable
private fun CustomizationSection(viewModel: SettingsViewModel) {
    val context = LocalContext.current

    SmallTitle(text = stringResource(CoreR.string.settings_customization))
    Card(modifier = Modifier.fillMaxWidth()) {
        if (LocaleSetting.useLocaleManager) {
            val locale = LocaleSetting.instance.appLocale
            val summary = locale?.getDisplayName(locale) ?: stringResource(CoreR.string.system_default)
            SuperArrow(
                title = stringResource(CoreR.string.language),
                summary = summary,
                onClick = {
                    context.startActivity(LocaleSetting.localeSettingsIntent)
                }
            )
        } else {
            val names = remember { LocaleSetting.available.names }
            val tags = remember { LocaleSetting.available.tags }
            var selectedIndex by remember {
                mutableIntStateOf(tags.indexOf(Config.locale).coerceAtLeast(0))
            }
            SuperDropdown(
                title = stringResource(CoreR.string.language),
                items = names.toList(),
                selectedIndex = selectedIndex,
                onSelectedIndexChange = { index ->
                    selectedIndex = index
                    Config.locale = tags[index]
                }
            )
        }

        if (isRunningAsStub && ShortcutManagerCompat.isRequestPinShortcutSupported(context)) {
            SuperArrow(
                title = stringResource(CoreR.string.add_shortcut_title),
                summary = stringResource(CoreR.string.setting_add_shortcut_summary),
                onClick = { viewModel.requestAddShortcut() }
            )
        }
    }
}

// --- App Settings ---

@Composable
private fun AppSettingsSection(viewModel: SettingsViewModel) {
    val context = LocalContext.current
    val resources = context.resources
    val hidden = context.packageName != BuildConfig.APP_PACKAGE_NAME

    SmallTitle(text = stringResource(CoreR.string.home_app_title))
    Card(modifier = Modifier.fillMaxWidth()) {
        // Update Channel
        val updateChannelEntries = remember {
            resources.getStringArray(CoreR.array.update_channel).toList()
        }
        var updateChannel by remember {
            mutableIntStateOf(Config.updateChannel.coerceIn(0, updateChannelEntries.size - 1))
        }
        var showUrlDialog by remember { mutableStateOf(false) }

        SuperDropdown(
            title = stringResource(CoreR.string.settings_update_channel_title),
            items = updateChannelEntries,
            selectedIndex = updateChannel,
            onSelectedIndexChange = { index ->
                updateChannel = index
                Config.updateChannel = index
                Info.resetUpdate()
                if (index == Config.Value.CUSTOM_CHANNEL && Config.customChannelUrl.isBlank()) {
                    showUrlDialog = true
                }
            }
        )

        // Update Channel URL (for custom channel)
        if (updateChannel == Config.Value.CUSTOM_CHANNEL) {
            UpdateChannelUrlDialog(
                show = showUrlDialog,
                onDismiss = { showUrlDialog = false }
            )
            SuperArrow(
                title = stringResource(CoreR.string.settings_update_custom),
                summary = Config.customChannelUrl.ifBlank { null },
                onClick = { showUrlDialog = true }
            )
        }

        // DoH Toggle
        var doh by remember { mutableStateOf(Config.doh) }
        SuperSwitch(
            title = stringResource(CoreR.string.settings_doh_title),
            summary = stringResource(CoreR.string.settings_doh_description),
            checked = doh,
            onCheckedChange = {
                doh = it
                Config.doh = it
            }
        )

        // Update Checker
        var checkUpdate by remember { mutableStateOf(Config.checkUpdate) }
        SuperSwitch(
            title = stringResource(CoreR.string.settings_check_update_title),
            summary = stringResource(CoreR.string.settings_check_update_summary),
            checked = checkUpdate,
            onCheckedChange = { newValue ->
                checkUpdate = newValue
                Config.checkUpdate = newValue
            }
        )

        // Download Path
        var showDownloadDialog by remember { mutableStateOf(false) }
        DownloadPathDialog(
            show = showDownloadDialog,
            onDismiss = { showDownloadDialog = false }
        )
        SuperArrow(
            title = stringResource(CoreR.string.settings_download_path_title),
            summary = MediaStoreUtils.fullPath(Config.downloadDir),
            onClick = {
                showDownloadDialog = true
            }
        )

        // Random Package Name
        var randName by remember { mutableStateOf(Config.randName) }
        SuperSwitch(
            title = stringResource(CoreR.string.settings_random_name_title),
            summary = stringResource(CoreR.string.settings_random_name_description),
            checked = randName,
            onCheckedChange = {
                randName = it
                Config.randName = it
            }
        )

        // Hide / Restore
        if (Info.env.isActive && Const.USER_ID == 0) {
            if (hidden) {
                var showRestoreDialog by remember { mutableStateOf(false) }
                RestoreDialog(
                    show = showRestoreDialog,
                    onDismiss = { showRestoreDialog = false },
                    onConfirm = {
                        showRestoreDialog = false
                        viewModel.restoreApp(context as Activity)
                    }
                )
                SuperArrow(
                    title = stringResource(CoreR.string.settings_restore_app_title),
                    summary = stringResource(CoreR.string.settings_restore_app_summary),
                    onClick = { showRestoreDialog = true }
                )
            } else {
                var showHideDialog by remember { mutableStateOf(false) }
                HideAppDialog(
                    show = showHideDialog,
                    onDismiss = { showHideDialog = false },
                    onConfirm = { name ->
                        showHideDialog = false
                        viewModel.hideApp(context as Activity, name)
                    }
                )
                SuperArrow(
                    title = stringResource(CoreR.string.settings_hide_app_title),
                    summary = stringResource(CoreR.string.settings_hide_app_summary),
                    onClick = { showHideDialog = true }
                )
            }
        }
    }
}

// --- Magisk ---

@Composable
private fun MagiskSection(viewModel: SettingsViewModel) {
    SmallTitle(text = stringResource(CoreR.string.magisk))
    Card(modifier = Modifier.fillMaxWidth()) {
        // Systemless Hosts
        SuperArrow(
            title = stringResource(CoreR.string.settings_hosts_title),
            summary = stringResource(CoreR.string.settings_hosts_summary),
            onClick = { viewModel.createHosts() }
        )

        if (Const.Version.atLeast_24_0()) {
            // Zygisk
            var zygisk by remember { mutableStateOf(Config.zygisk) }
            SuperSwitch(
                title = stringResource(CoreR.string.zygisk),
                summary = stringResource(
                    if (zygisk != Info.isZygiskEnabled) CoreR.string.reboot_apply_change
                    else CoreR.string.settings_zygisk_summary
                ),
                checked = zygisk,
                onCheckedChange = {
                    zygisk = it
                    Config.zygisk = it
                    viewModel.notifyZygiskChange()
                }
            )

            // DenyList
            val denyListEnabled by viewModel.denyListEnabled.collectAsState()
            SuperSwitch(
                title = stringResource(CoreR.string.settings_denylist_title),
                summary = stringResource(CoreR.string.settings_denylist_summary),
                checked = denyListEnabled,
                onCheckedChange = { viewModel.toggleDenyList(it) }
            )

            // DenyList Config
            SuperArrow(
                title = stringResource(CoreR.string.settings_denylist_config_title),
                summary = stringResource(CoreR.string.settings_denylist_config_summary),
                onClick = { viewModel.navigateToDenyList() }
            )
        }
    }
}

// --- Superuser ---

@Composable
private fun SuperuserSection(viewModel: SettingsViewModel) {
    val context = LocalContext.current
    val resources = context.resources

    SmallTitle(text = stringResource(CoreR.string.superuser))
    Card(modifier = Modifier.fillMaxWidth()) {
        // Tapjack (SDK < S)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            var tapjack by remember { mutableStateOf(Config.suTapjack) }
            SuperSwitch(
                title = stringResource(CoreR.string.settings_su_tapjack_title),
                summary = stringResource(CoreR.string.settings_su_tapjack_summary),
                checked = tapjack,
                onCheckedChange = {
                    tapjack = it
                    Config.suTapjack = it
                }
            )
        }

        // Authentication
        var suAuth by remember { mutableStateOf(Config.suAuth) }
        SuperSwitch(
            title = stringResource(CoreR.string.settings_su_auth_title),
            summary = stringResource(
                if (Info.isDeviceSecure) CoreR.string.settings_su_auth_summary
                else CoreR.string.settings_su_auth_insecure
            ),
            checked = suAuth,
            enabled = Info.isDeviceSecure,
            onCheckedChange = { newValue ->
                viewModel.withAuth {
                    suAuth = newValue
                    Config.suAuth = newValue
                }
            }
        )

        // Access Mode
        val accessEntries = remember {
            resources.getStringArray(CoreR.array.su_access).toList()
        }
        var accessMode by remember { mutableIntStateOf(Config.rootMode) }
        SuperDropdown(
            title = stringResource(CoreR.string.superuser_access),
            items = accessEntries,
            selectedIndex = accessMode,
            onSelectedIndexChange = {
                accessMode = it
                Config.rootMode = it
            }
        )

        // Multiuser Mode
        val multiuserEntries = remember {
            resources.getStringArray(CoreR.array.multiuser_mode).toList()
        }
        val multiuserDescriptions = remember {
            resources.getStringArray(CoreR.array.multiuser_summary).toList()
        }
        var multiuserMode by remember { mutableIntStateOf(Config.suMultiuserMode) }
        SuperDropdown(
            title = stringResource(CoreR.string.multiuser_mode),
            summary = multiuserDescriptions.getOrElse(multiuserMode) { "" },
            items = multiuserEntries,
            selectedIndex = multiuserMode,
            enabled = Const.USER_ID == 0,
            onSelectedIndexChange = {
                multiuserMode = it
                Config.suMultiuserMode = it
            }
        )

        // Mount Namespace Mode
        val namespaceEntries = remember {
            resources.getStringArray(CoreR.array.namespace).toList()
        }
        val namespaceDescriptions = remember {
            resources.getStringArray(CoreR.array.namespace_summary).toList()
        }
        var mntNamespaceMode by remember { mutableIntStateOf(Config.suMntNamespaceMode) }
        SuperDropdown(
            title = stringResource(CoreR.string.mount_namespace_mode),
            summary = namespaceDescriptions.getOrElse(mntNamespaceMode) { "" },
            items = namespaceEntries,
            selectedIndex = mntNamespaceMode,
            onSelectedIndexChange = {
                mntNamespaceMode = it
                Config.suMntNamespaceMode = it
            }
        )

        // Automatic Response
        val autoResponseEntries = remember {
            resources.getStringArray(CoreR.array.auto_response).toList()
        }
        var autoResponse by remember { mutableIntStateOf(Config.suAutoResponse) }
        SuperDropdown(
            title = stringResource(CoreR.string.auto_response),
            items = autoResponseEntries,
            selectedIndex = autoResponse,
            onSelectedIndexChange = { newIndex ->
                val doIt = {
                    autoResponse = newIndex
                    Config.suAutoResponse = newIndex
                }
                if (Config.suAuth) viewModel.withAuth(doIt) else doIt()
            }
        )

        // Request Timeout
        val timeoutEntries = remember {
            resources.getStringArray(CoreR.array.request_timeout).toList()
        }
        val timeoutValues = remember { listOf(10, 15, 20, 30, 45, 60) }
        var timeoutIndex by remember {
            mutableIntStateOf(timeoutValues.indexOf(Config.suDefaultTimeout).coerceAtLeast(0))
        }
        SuperDropdown(
            title = stringResource(CoreR.string.request_timeout),
            items = timeoutEntries,
            selectedIndex = timeoutIndex,
            onSelectedIndexChange = {
                timeoutIndex = it
                Config.suDefaultTimeout = timeoutValues[it]
            }
        )

        // SU Notification
        val notifEntries = remember {
            resources.getStringArray(CoreR.array.su_notification).toList()
        }
        var suNotification by remember { mutableIntStateOf(Config.suNotification) }
        SuperDropdown(
            title = stringResource(CoreR.string.superuser_notification),
            items = notifEntries,
            selectedIndex = suNotification,
            onSelectedIndexChange = {
                suNotification = it
                Config.suNotification = it
            }
        )

        // Reauthenticate (SDK < O)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            var reAuth by remember { mutableStateOf(Config.suReAuth) }
            SuperSwitch(
                title = stringResource(CoreR.string.settings_su_reauth_title),
                summary = stringResource(CoreR.string.settings_su_reauth_summary),
                checked = reAuth,
                onCheckedChange = {
                    reAuth = it
                    Config.suReAuth = it
                }
            )
        }

        // Restrict (version >= 30.1)
        if (Const.Version.atLeast_30_1()) {
            var restrict by remember { mutableStateOf(Config.suRestrict) }
            SuperSwitch(
                title = stringResource(CoreR.string.settings_su_restrict_title),
                summary = stringResource(CoreR.string.settings_su_restrict_summary),
                checked = restrict,
                onCheckedChange = {
                    restrict = it
                    Config.suRestrict = it
                }
            )
        }
    }
}

// --- Dialogs ---

@Composable
private fun UpdateChannelUrlDialog(show: Boolean, onDismiss: () -> Unit) {
    val showState = rememberSaveable { mutableStateOf(show) }
    showState.value = show
    var url by rememberSaveable { mutableStateOf(Config.customChannelUrl) }

    SuperDialog(
        show = showState,
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            TextField(
                value = url,
                onValueChange = { url = it },
                modifier = Modifier.fillMaxWidth(),
                label = stringResource(CoreR.string.settings_update_custom_msg)
            )
            Spacer(Modifier.height(16.dp))
            TextButton(
                text = stringResource(android.R.string.ok),
                onClick = {
                    Config.customChannelUrl = url
                    Info.resetUpdate()
                    onDismiss()
                },
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
private fun DownloadPathDialog(show: Boolean, onDismiss: () -> Unit) {
    val showState = rememberSaveable { mutableStateOf(show) }
    showState.value = show
    var path by rememberSaveable { mutableStateOf(Config.downloadDir) }

    SuperDialog(
        show = showState,
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            top.yukonga.miuix.kmp.basic.Text(
                text = stringResource(CoreR.string.settings_download_path_message, MediaStoreUtils.fullPath(path)),
                modifier = Modifier.padding(bottom = 8.dp)
            )
            TextField(
                value = path,
                onValueChange = { path = it },
                modifier = Modifier.fillMaxWidth(),
                label = stringResource(CoreR.string.settings_download_path_title)
            )
            Spacer(Modifier.height(16.dp))
            TextButton(
                text = stringResource(android.R.string.ok),
                onClick = {
                    Config.downloadDir = path
                    onDismiss()
                },
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
private fun HideAppDialog(show: Boolean, onDismiss: () -> Unit, onConfirm: (String) -> Unit) {
    val showState = rememberSaveable { mutableStateOf(show) }
    showState.value = show
    var appName by rememberSaveable { mutableStateOf("Settings") }
    val isError = appName.length > AppMigration.MAX_LABEL_LENGTH || appName.isBlank()

    SuperDialog(
        show = showState,
        title = stringResource(CoreR.string.settings_hide_app_title),
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            TextField(
                value = appName,
                onValueChange = { appName = it },
                modifier = Modifier.fillMaxWidth(),
                label = stringResource(CoreR.string.settings_app_name_hint),
            )
            Spacer(Modifier.height(16.dp))
            Row(
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                TextButton(
                    text = stringResource(android.R.string.cancel),
                    onClick = onDismiss,
                    modifier = Modifier.weight(1f)
                )
                Spacer(Modifier.width(20.dp))
                TextButton(
                    text = stringResource(android.R.string.ok),
                    onClick = { if (!isError) onConfirm(appName) },
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.textButtonColorsPrimary()
                )
            }
        }
    }
}

@Composable
private fun RestoreDialog(show: Boolean, onDismiss: () -> Unit, onConfirm: () -> Unit) {
    val showState = rememberSaveable { mutableStateOf(show) }
    showState.value = show

    SuperDialog(
        show = showState,
        title = stringResource(CoreR.string.settings_restore_app_title),
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            top.yukonga.miuix.kmp.basic.Text(
                text = stringResource(CoreR.string.restore_app_confirmation),
                color = MiuixTheme.colorScheme.onSurface,
            )
            Spacer(Modifier.height(16.dp))
            Row(
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                TextButton(
                    text = stringResource(android.R.string.cancel),
                    onClick = onDismiss,
                    modifier = Modifier.weight(1f)
                )
                Spacer(Modifier.width(20.dp))
                TextButton(
                    text = stringResource(android.R.string.ok),
                    onClick = onConfirm,
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.textButtonColorsPrimary()
                )
            }
        }
    }
}
