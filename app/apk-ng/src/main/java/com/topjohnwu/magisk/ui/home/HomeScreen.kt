package com.topjohnwu.magisk.ui.home

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.PowerManager
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.ui.graphics.Color
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.DpSize
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.getSystemService
import androidx.core.net.toUri
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.LoadingDialogHandle
import com.topjohnwu.magisk.ui.component.MarkdownText
import com.topjohnwu.magisk.ui.component.MarkdownTextAsync
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.rememberLoadingDialog
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.install.InstallViewModel
import kotlinx.coroutines.launch
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.PushPin
import androidx.compose.material.icons.filled.Layers
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.VerticalDivider
import com.topjohnwu.magisk.ui.component.SettingsArrow
import androidx.compose.material.icons.filled.Visibility
import androidx.compose.material.icons.filled.VisibilityOff
import com.topjohnwu.magisk.core.R as CoreR


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(viewModel: HomeViewModel, installVm: InstallViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val installUiState by installVm.uiState.collectAsState()
    val context = LocalContext.current
    val activity = context as MainActivity
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val scope = rememberCoroutineScope()
    val loadingDialog = rememberLoadingDialog()
    val navigator = com.topjohnwu.magisk.ui.navigation.LocalNavigator.current

    val showUninstallDialog = rememberSaveable { mutableStateOf(false) }
    val showManagerDialog = rememberSaveable { mutableStateOf(false) }
    val showEnvFixDialog = rememberSaveable { mutableStateOf(false) }
    var showHideDialog by rememberSaveable { mutableStateOf(false) }
    var showRestoreDialog by rememberSaveable { mutableStateOf(false) }
    val showInstallSheet = rememberSaveable { mutableStateOf(false) }
    var envFixCode by remember { mutableIntStateOf(0) }

    val filePicker = rememberLauncherForActivityResult(ActivityResultContracts.GetContent()) { uri ->
        uri?.let { installVm.onPatchFileSelected(it) }
    }

    val secondSlotDialog = rememberConfirmDialog()
    val secondSlotTitle = stringResource(android.R.string.dialog_alert_title)
    val secondSlotMsg = stringResource(CoreR.string.install_inactive_slot_msg)

    LaunchedEffect(installUiState.requestFilePicker) {
        if (installUiState.requestFilePicker) {
            filePicker.launch("*/*")
            installVm.onFilePickerConsumed()
        }
    }

    LaunchedEffect(installUiState.showSecondSlotWarning) {
        if (installUiState.showSecondSlotWarning) {
            val result = secondSlotDialog.awaitConfirm(title = secondSlotTitle, content = secondSlotMsg)
            installVm.onSecondSlotWarningConsumed()
            if (result == ConfirmResult.Confirmed) {
                installVm.install()
            }
        }
    }

    LaunchedEffect(uiState.showUninstall) {
        if (uiState.showUninstall) {
            showUninstallDialog.value = true
            viewModel.onUninstallConsumed()
        }
    }
    LaunchedEffect(uiState.showManagerInstall) {
        if (uiState.showManagerInstall) {
            showManagerDialog.value = true
            viewModel.onManagerInstallConsumed()
        }
    }
    LaunchedEffect(uiState.envFixCode) {
        if (uiState.envFixCode != 0) {
            envFixCode = uiState.envFixCode
            showEnvFixDialog.value = true
            viewModel.onEnvFixConsumed()
        }
    }
    LaunchedEffect(uiState.showHideRestore) {
        if (uiState.showHideRestore) {
            val hidden = context.packageName != BuildConfig.APP_PACKAGE_NAME
            if (hidden) showRestoreDialog = true else showHideDialog = true
            viewModel.onHideRestoreConsumed()
        }
    }

    if (showUninstallDialog.value) {
        UninstallComposableDialog(
            showDialog = showUninstallDialog,
            activity = activity,
            loadingDialog = loadingDialog,
        )
    }

    if (showManagerDialog.value) {
        ManagerInstallComposableDialog(
            showDialog = showManagerDialog,
            activity = activity,
        )
    }

    if (showEnvFixDialog.value) {
        EnvFixComposableDialog(
            showDialog = showEnvFixDialog,
            code = envFixCode,
            activity = activity,
            loadingDialog = loadingDialog,
            onNavigateInstall = { showInstallSheet.value = true },
        )
    }

    if (showHideDialog) {
        HideAppDialog(
            onDismiss = { showHideDialog = false },
            onConfirm = { name ->
                showHideDialog = false
                scope.launch {
                    loadingDialog.withLoading {
                        AppMigration.patchAndHide(context, name)
                    }
                }
            }
        )
    }

    if (showRestoreDialog) {
        RestoreAppDialog(
            onDismiss = { showRestoreDialog = false },
            onConfirm = {
                showRestoreDialog = false
                scope.launch {
                    loadingDialog.withLoading {
                        AppMigration.restoreApp(context)
                    }
                }
            }
        )
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.section_home)) },
                scrollBehavior = scrollBehavior,
                actions = {
                    if (Info.isRooted) {
                        RebootButton()
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp)
                .padding(top = 12.dp, bottom = 88.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (uiState.isNoticeVisible) {
                NoticeCard(onHide = viewModel::hideNotice)
            }

            CoreCard(
                modifier = Modifier.fillMaxWidth(),
                state = viewModel.magiskState,
                version = viewModel.magiskInstalledVersion,
                remoteVersion = if (viewModel.magiskState == HomeViewModel.State.OUTDATED)
                    "${BuildConfig.APP_VERSION_NAME} (${BuildConfig.APP_VERSION_CODE})" else null,
                onInstallClicked = { showInstallSheet.value = true },
                onUninstallClicked = { viewModel.onDeletePressed() },
            )

            UninstallButton(
                onClick = { viewModel.onDeletePressed() },
                enabled = Info.env.isActive
            )

            StatusCard()

            AppCard(
                modifier = Modifier.fillMaxWidth(),
                state = uiState.appState,
                version = viewModel.managerInstalledVersion,
                remoteVersion = if (uiState.appState == HomeViewModel.State.OUTDATED)
                    uiState.managerRemoteVersion else null,
                progress = uiState.managerProgress,
                isHidden = context.packageName != BuildConfig.APP_PACKAGE_NAME,
                onManagerPressed = { viewModel.onManagerPressed() },
                onHideRestorePressed = viewModel::onHideRestorePressed,
            )

            val showDonateSheet = rememberSaveable { mutableStateOf(false) }

            Text(
                text = stringResource(CoreR.string.home_support_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(start = 16.dp, top = 8.dp, bottom = 4.dp)
            )
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(24.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = stringResource(CoreR.string.home_support_content),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    Spacer(Modifier.height(16.dp))
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceEvenly,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        IconButton(onClick = { viewModel.onLinkPressed(Const.Url.PATREON_URL) }) {
                            Icon(
                                painter = painterResource(CoreR.drawable.ic_patreon),
                                contentDescription = "Patreon",
                                modifier = Modifier.size(32.dp)
                            )
                        }
                        IconButton(onClick = { viewModel.onLinkPressed("https://paypal.me/magiskdonate") }) {
                            Icon(
                                painter = painterResource(CoreR.drawable.ic_paypal),
                                contentDescription = "PayPal",
                                modifier = Modifier.size(32.dp)
                            )
                        }
                    }
                }
            }

            Text(
                text = stringResource(CoreR.string.home_follow_title),
                style = MaterialTheme.typography.titleSmall,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(start = 24.dp, top = 24.dp, bottom = 8.dp)
            )
            DevelopersCard(onLinkClicked = { openLink(context, it) })
        }
    }

    InstallBottomSheet(
        show = showInstallSheet,
        installVm = installVm,
        installUiState = installUiState,
    )
}

@Composable
private fun RebootButton() {
    val showMenu = remember { mutableStateOf(false) }
    val context = LocalContext.current
    var safeModeEnabled by remember { mutableIntStateOf(Config.bootloop) }

    val showUserspace = Build.VERSION.SDK_INT >= Build.VERSION_CODES.R &&
        context.getSystemService<PowerManager>()?.isRebootingUserspaceSupported == true
    val showSafeMode = Const.Version.atLeast_28_0()

    val items = buildList {
        add(RebootOption(CoreR.string.reboot) { reboot() })
        if (showUserspace) {
            add(RebootOption(CoreR.string.reboot_userspace) { reboot("userspace") })
        }
        add(RebootOption(CoreR.string.reboot_recovery) { reboot("recovery") })
        add(RebootOption(CoreR.string.reboot_bootloader) { reboot("bootloader") })
        add(RebootOption(CoreR.string.reboot_download) { reboot("download") })
        add(RebootOption(CoreR.string.reboot_edl) { reboot("edl") })
        if (showSafeMode) {
            add(RebootOption(CoreR.string.reboot_safe_mode) {
                val newVal = if (safeModeEnabled >= 2) 0 else 2
                Config.bootloop = newVal
                safeModeEnabled = newVal
            })
        }
    }

    Box {
        IconButton(
            modifier = Modifier.padding(end = 16.dp),
            onClick = { showMenu.value = true },
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_restart),
                contentDescription = stringResource(CoreR.string.reboot),
            )
        }
        DropdownMenu(
            expanded = showMenu.value,
            onDismissRequest = { showMenu.value = false }
        ) {
            items.forEachIndexed { index, item ->
                val isSafeMode = item.labelRes == CoreR.string.reboot_safe_mode
                DropdownMenuItem(
                    text = { Text(stringResource(item.labelRes)) },
                    trailingIcon = if (isSafeMode && safeModeEnabled >= 2) {
                        { Icon(Icons.Default.Check, contentDescription = null) }
                    } else null,
                    onClick = {
                        item.action()
                        if (!isSafeMode) showMenu.value = false
                    }
                )
            }
        }
    }
}

private class RebootOption(val labelRes: Int, val action: () -> Unit)

@Composable
private fun NoticeCard(onHide: () -> Unit) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .background(
                MaterialTheme.colorScheme.tertiaryContainer,
                RoundedCornerShape(16.dp)
            )
            .padding(start = 16.dp, top = 4.dp, bottom = 4.dp, end = 4.dp)
    ) {
        Row(
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(CoreR.string.home_notice_content),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onTertiaryContainer,
                modifier = Modifier.weight(1f).padding(vertical = 8.dp)
            )
            IconButton(onClick = onHide) {
                Icon(
                    imageVector = Icons.Default.Close,
                    contentDescription = stringResource(CoreR.string.hide),
                    modifier = Modifier.size(15.dp),
                    tint = MaterialTheme.colorScheme.onTertiaryContainer,
                )
            }
        }
    }
}

@Composable
private fun CoreCard(
    modifier: Modifier = Modifier,
    state: HomeViewModel.State,
    version: String,
    remoteVersion: String? = null,
    onInstallClicked: () -> Unit,
    onUninstallClicked: () -> Unit,
) {
    val actionLabel = when (state) {
        HomeViewModel.State.OUTDATED -> stringResource(CoreR.string.update)
        HomeViewModel.State.INVALID -> stringResource(CoreR.string.install)
        HomeViewModel.State.UP_TO_DATE -> stringResource(CoreR.string.reinstall)
        HomeViewModel.State.LOADING -> null
    }

    Card(
        modifier = modifier,
        shape = RoundedCornerShape(24.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.weight(1f)
            ) {
                Icon(
                    painter = painterResource(CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier.size(48.dp)
                )
                Spacer(Modifier.width(16.dp))
                Column {
                    Text(
                        text = stringResource(CoreR.string.home_core_title),
                        style = MaterialTheme.typography.titleLarge
                    )
                    Text(
                        text = version.ifEmpty { stringResource(CoreR.string.not_available) },
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            if (actionLabel != null) {
                Button(
                    onClick = onInstallClicked,
                    colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary),
                    shape = RoundedCornerShape(16.dp),
                    contentPadding = PaddingValues(horizontal = 12.dp, vertical = 8.dp)
                ) {
                    Icon(
                        painter = painterResource(R.drawable.ic_download_md2),
                        contentDescription = null,
                        modifier = Modifier.size(18.dp),
                        tint = Color.White
                    )
                    Spacer(Modifier.width(4.dp))
                    Text(
                        text = actionLabel,
                        color = Color.White,
                        fontSize = 14.sp
                    )
                }
            }
        }
    }
}

@Composable
private fun UninstallButton(
    onClick: () -> Unit,
    enabled: Boolean
) {
    Button(
        onClick = onClick,
        enabled = enabled,
        colors = ButtonDefaults.buttonColors(
            containerColor = MaterialTheme.colorScheme.errorContainer,
            contentColor = MaterialTheme.colorScheme.onErrorContainer
        ),
        shape = RoundedCornerShape(16.dp),
        modifier = Modifier.fillMaxWidth()
    ) {
        Icon(
            imageVector = Icons.Default.Delete,
            contentDescription = null,
            modifier = Modifier.size(18.dp)
        )
        Spacer(Modifier.width(8.dp))
        Text(
            text = stringResource(CoreR.string.uninstall_magisk_title),
            fontSize = 16.sp
        )
    }
}

@Composable
private fun AppCard(
    modifier: Modifier = Modifier,
    state: HomeViewModel.State,
    version: String,
    remoteVersion: String? = null,
    progress: Int,
    isHidden: Boolean,
    onManagerPressed: () -> Unit,
    onHideRestorePressed: () -> Unit,
) {
    val actionLabel = when (state) {
        HomeViewModel.State.OUTDATED -> stringResource(CoreR.string.update)
        HomeViewModel.State.UP_TO_DATE -> stringResource(CoreR.string.reinstall)
        else -> null
    }

    Card(
        modifier = modifier,
        shape = RoundedCornerShape(24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        painter = painterResource(R.drawable.ic_manager),
                        contentDescription = null,
                        modifier = Modifier.size(36.dp)
                    )
                    Spacer(Modifier.width(12.dp))
                    Text(
                        text = stringResource(CoreR.string.home_app_title),
                        style = MaterialTheme.typography.titleLarge
                    )
                }

                if (Info.env.isActive) {
                    val hideRestoreIcon = if (isHidden) Icons.Default.Visibility else Icons.Default.VisibilityOff
                    Spacer(modifier = Modifier.weight(1f))
                    IconButton(onClick = onHideRestorePressed) {
                        Icon(
                            imageVector = hideRestoreIcon,
                            contentDescription = null,
                            modifier = Modifier.size(18.dp),
                            tint = Color.White
                        )
                    }
                }

                if (actionLabel != null) {
                    Button(
                        onClick = onManagerPressed,
                        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary),
                        shape = RoundedCornerShape(16.dp),
                        contentPadding = PaddingValues(horizontal = 12.dp, vertical = 8.dp)
                    ) {
                        Icon(
                            painter = painterResource(R.drawable.ic_update_md2),
                            contentDescription = null,
                            modifier = Modifier.size(18.dp),
                            tint = Color.White
                        )
                        Spacer(Modifier.width(4.dp))
                        Text(
                            text = actionLabel,
                            color = Color.White,
                            fontSize = 14.sp
                        )
                    }
                }
            }

            Spacer(Modifier.height(16.dp))

            AppDetailRow(label = stringResource(CoreR.string.home_latest_version), value = remoteVersion ?: version)
            AppDetailRow(label = stringResource(CoreR.string.home_installed_version), value = version)
            AppDetailRow(label = stringResource(CoreR.string.home_package), value = LocalContext.current.packageName)

            if (progress in 1..99) {
                Spacer(Modifier.height(8.dp))
                LinearProgressIndicator(
                    progress = progress / 100f,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}

@Composable
private fun AppDetailRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(text = label, style = MaterialTheme.typography.bodyMedium)
        Text(text = value, style = MaterialTheme.typography.bodyMedium)
    }
}

@Composable
private fun UpdateBadge(version: String, modifier: Modifier = Modifier) {
    Text(
        text = version,
        color = MaterialTheme.colorScheme.onPrimary,
        fontSize = 10.sp,
        maxLines = 1,
        modifier = modifier
            .background(MaterialTheme.colorScheme.primary, RoundedCornerShape(6.dp))
            .padding(horizontal = 6.dp, vertical = 2.dp)
    )
}

@Composable
private fun StatusCard() {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        val ZygiskEnabled = Info.isZygiskEnabled
        val ZygiskLabel = stringResource(CoreR.string.zygisk)
        val ZygiskStatus = stringResource(if (ZygiskEnabled) CoreR.string.yes else CoreR.string.no)

        val ramdiskEnabled = Info.ramdisk
        val ramdiskLabel = stringResource(CoreR.string.ramdisk)
        val ramdiskStatus = stringResource(if (ramdiskEnabled) CoreR.string.yes else CoreR.string.no)

        StatusItemCard(
            modifier = Modifier.weight(1f),
            icon = Icons.Filled.PushPin,
            text = "$ZygiskLabel $ZygiskStatus"
        )
        StatusItemCard(
            modifier = Modifier.weight(1f),
            icon = Icons.Filled.Layers,
            text = "$ramdiskLabel $ramdiskStatus"
        )
    }
}

@Composable
private fun StatusItemCard(
    modifier: Modifier = Modifier,
    icon: androidx.compose.ui.graphics.vector.ImageVector,
    text: String
) {
    Card(
        modifier = modifier,
        shape = RoundedCornerShape(20.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                modifier = Modifier.size(24.dp).rotate(if (icon == Icons.Filled.PushPin) 45f else 0f)
            )
            Spacer(Modifier.width(8.dp))
            Text(
                text = text,
                style = MaterialTheme.typography.bodyLarge
            )
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SupportBottomSheet(
    show: MutableState<Boolean>,
    onLinkClicked: (String) -> Unit,
) {
    if (show.value) {
        ModalBottomSheet(
            onDismissRequest = { show.value = false },
        ) {
            Text(
                text = stringResource(CoreR.string.home_support_title),
                style = MaterialTheme.typography.titleLarge,
                modifier = Modifier.padding(16.dp)
            )
            Column(modifier = Modifier.padding(bottom = 16.dp)) {
                Text(
                    text = stringResource(CoreR.string.home_support_content),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
                )
                SettingsArrow(
                    title = stringResource(CoreR.string.patreon),
                    onClick = {
                        show.value = false
                        onLinkClicked(Const.Url.PATREON_URL)
                    },
                    leadingContent = {
                        Icon(
                            painter = painterResource(CoreR.drawable.ic_patreon),
                            contentDescription = null,
                            modifier = Modifier.size(24.dp),
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                )
                SettingsArrow(
                    title = stringResource(CoreR.string.paypal),
                    onClick = {
                        show.value = false
                        onLinkClicked("https://paypal.me/magiskdonate")
                    },
                    leadingContent = {
                        Icon(
                            painter = painterResource(CoreR.drawable.ic_paypal),
                            contentDescription = null,
                            modifier = Modifier.size(24.dp),
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                )
            }
        }
    }
}

private data class LinkInfo(val label: String, val icon: Int, val url: String)
private data class DeveloperInfo(val name: String, val links: List<LinkInfo>)

private val developers = listOf(
    DeveloperInfo("topjohnwu", listOf(
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/topjohnwu"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, Const.Url.SOURCE_CODE_URL),
    )),
    DeveloperInfo("vvb2060", listOf(
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/vvb2060"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/vvb2060"),
    )),
    DeveloperInfo("yujincheng08", listOf(
        LinkInfo("Sponsor", CoreR.drawable.ic_favorite, "https://github.com/sponsors/yujincheng08"),
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/shanasaimoe"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/yujincheng08"),
    )),
    DeveloperInfo("rikkawww", listOf(
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/rikkawww"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/rikkawww"),
    )),
    DeveloperInfo("canyie", listOf(
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/canyie2977"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/canyie"),
    )),
)

@Composable
private fun DevelopersCard(onLinkClicked: (String) -> Unit) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(24.dp)
    ) {
        Column {
            developers.forEachIndexed { index, dev ->
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp, vertical = 8.dp),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = "@${dev.name}",
                        style = MaterialTheme.typography.bodyLarge
                    )
                    Row(
                        horizontalArrangement = Arrangement.spacedBy(4.dp)
                    ) {
                        dev.links.forEach { link ->
                            IconButton(onClick = { onLinkClicked(link.url) }) {
                                Icon(
                                    painter = painterResource(link.icon),
                                    contentDescription = link.label,
                                    modifier = Modifier.size(24.dp)
                                )
                            }
                        }
                    }
                }
                if (index < developers.lastIndex) {
                    HorizontalDivider(
                        thickness = 0.5.dp,
                        modifier = Modifier.padding(horizontal = 16.dp)
                    )
                }
            }
        }
    }
}

private fun openLink(context: Context, url: String) {
    try {
        context.startActivity(Intent(Intent.ACTION_VIEW, url.toUri()).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        })
    } catch (_: ActivityNotFoundException) { }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun InstallBottomSheet(
    show: MutableState<Boolean>,
    installVm: InstallViewModel,
    installUiState: InstallViewModel.UiState,
) {
    if (show.value) {
        ModalBottomSheet(
            onDismissRequest = { show.value = false },
        ) {
            Text(
                text = stringResource(CoreR.string.install),
                style = MaterialTheme.typography.titleLarge,
                modifier = Modifier.padding(16.dp)
            )
        Column(modifier = Modifier.padding(bottom = 16.dp)) {
            if (installUiState.notes.isNotEmpty()) {
                Box(modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)) {
                    MarkdownText(installUiState.notes)
                }
                HorizontalDivider(thickness = 0.75.dp)
            }

            if (!installVm.skipOptions) {
                InstallOptionsSection(installUiState, installVm)
            }

            SettingsArrow(
                title = stringResource(CoreR.string.select_patch_file),
                summary = stringResource(CoreR.string.select_patch_file_summary),
                onClick = {
                    show.value = false
                    installVm.selectMethod(InstallViewModel.Method.PATCH)
                },
                // enabled = installUiState.step >= 1 || installVm.skipOptions
            )

            if (installVm.isRooted) {
                SettingsArrow(
                    title = stringResource(CoreR.string.direct_install),
                    summary = stringResource(CoreR.string.direct_install_summary),
                    onClick = {
                        show.value = false
                        installVm.selectMethod(InstallViewModel.Method.DIRECT)
                        installVm.install()
                    },
                    // enabled = installUiState.step >= 1 || installVm.skipOptions
                )
            }

            if (!installVm.noSecondSlot) {
                SettingsArrow(
                    title = stringResource(CoreR.string.install_inactive_slot),
                    summary = stringResource(CoreR.string.install_inactive_slot_summary),
                    onClick = {
                        show.value = false
                        installVm.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                    },
                    // enabled = installUiState.step >= 1 || installVm.skipOptions
                )
            }
        }
        }
    }
}

@Composable
private fun InstallOptionsSection(
    uiState: InstallViewModel.UiState,
    viewModel: InstallViewModel
) {
    Column(modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(CoreR.string.install_options_title),
                style = MaterialTheme.typography.titleLarge,
            )
            if (uiState.step == 0) {
                TextButton(
                    onClick = { viewModel.nextStep() }
                ) {
                    Text(stringResource(CoreR.string.install_next))
                }
            }
        }

        if (uiState.step == 0) {
            Spacer(Modifier.height(8.dp))
            if (!Info.isSAR) {
                CheckboxRow(
                    label = stringResource(CoreR.string.keep_dm_verity),
                    checked = Config.keepVerity,
                    onCheckedChange = { Config.keepVerity = it }
                )
            }
            if (Info.isFDE) {
                CheckboxRow(
                    label = stringResource(CoreR.string.keep_force_encryption),
                    checked = Config.keepEnc,
                    onCheckedChange = { Config.keepEnc = it }
                )
            }
            if (!Info.ramdisk) {
                CheckboxRow(
                    label = stringResource(CoreR.string.recovery_mode),
                    checked = Config.recovery,
                    onCheckedChange = { Config.recovery = it }
                )
            }
        }
    }
}

@Composable
private fun CheckboxRow(label: String, checked: Boolean, onCheckedChange: (Boolean) -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Checkbox(
            checked = checked,
            onCheckedChange = { onCheckedChange(it) }
        )
        Text(
            text = label,
            style = MaterialTheme.typography.bodyLarge,
        )
    }
}

@Composable
private fun UninstallComposableDialog(
    showDialog: MutableState<Boolean>,
    activity: MainActivity,
    loadingDialog: LoadingDialogHandle,
) {
    val scope = rememberCoroutineScope()
    if (showDialog.value) {
        AlertDialog(
            onDismissRequest = { showDialog.value = false },
            title = { Text(stringResource(CoreR.string.uninstall_magisk_title)) },
            text = {
                Text(
                    text = stringResource(CoreR.string.uninstall_magisk_msg),
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                )
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        showDialog.value = false
                        val intent = Intent(activity, activity.javaClass).apply {
                            action = FlashUtils.INTENT_FLASH
                            putExtra(FlashUtils.EXTRA_FLASH_ACTION, Const.Value.UNINSTALL)
                            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP
                        }
                        activity.startActivity(intent)
                    }
                ) {
                    Text(stringResource(CoreR.string.complete_uninstall))
                }
            },
            dismissButton = {
                TextButton(
                    onClick = {
                        showDialog.value = false
                        scope.launch {
                            val success = loadingDialog.withLoading {
                                MagiskInstaller.Restore().exec()
                            }
                            activity.toast(
                                if (success) CoreR.string.restore_done else CoreR.string.restore_fail,
                                Toast.LENGTH_SHORT
                            )
                        }
                    }
                ) {
                    Text(stringResource(CoreR.string.restore_img))
                }
            }
        )
    }
}

@Composable
private fun ManagerInstallComposableDialog(
    showDialog: MutableState<Boolean>,
    activity: MainActivity,
) {
    if (showDialog.value) {
        AlertDialog(
            onDismissRequest = { showDialog.value = false },
            title = { Text(stringResource(CoreR.string.install)) },
            text = {
                MarkdownTextAsync {
                    val text = Info.update.note
                    java.io.File(activity.cacheDir, "${Info.update.versionCode}.md").writeText(text)
                    text
                }
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        showDialog.value = false
                        DownloadEngine.startWithActivity(activity, Subject.App())
                    }
                ) {
                    Text(stringResource(CoreR.string.install))
                }
            },
            dismissButton = {
                TextButton(onClick = { showDialog.value = false }) {
                    Text(stringResource(android.R.string.cancel))
                }
            }
        )
    }
}

@Composable
private fun EnvFixComposableDialog(
    showDialog: MutableState<Boolean>,
    code: Int,
    activity: MainActivity,
    loadingDialog: LoadingDialogHandle,
    onNavigateInstall: () -> Unit,
) {
    val scope = rememberCoroutineScope()
    val needsFullFix = code == 2 ||
        Info.env.versionCode != com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE ||
        Info.env.versionString != com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_NAME

    if (showDialog.value) {
        AlertDialog(
            onDismissRequest = { showDialog.value = false },
            title = { Text(stringResource(CoreR.string.env_fix_title)) },
            text = {
                Text(
                    text = stringResource(
                        if (needsFullFix) CoreR.string.env_full_fix_msg else CoreR.string.env_fix_msg
                    ),
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                )
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        showDialog.value = false
                        if (needsFullFix) {
                            onNavigateInstall()
                        } else {
                            scope.launch {
                                val success = loadingDialog.withLoading {
                                    MagiskInstaller.FixEnv().exec()
                                }
                                activity.toast(
                                    if (success) CoreR.string.reboot_delay_toast else CoreR.string.setup_fail,
                                    Toast.LENGTH_LONG
                                )
                                if (success) {
                                    @Suppress("DEPRECATION")
                                    android.os.Handler(android.os.Looper.getMainLooper())
                                        .postDelayed({ reboot() }, 5000)
                                }
                            }
                        }
                    }
                ) {
                    Text(stringResource(android.R.string.ok))
                }
            },
            dismissButton = {
                TextButton(onClick = { showDialog.value = false }) {
                    Text(stringResource(android.R.string.cancel))
                }
            }
        )
    }
}

@Composable
private fun HideAppDialog(onDismiss: () -> Unit, onConfirm: (String) -> Unit) {
    val showState = rememberSaveable { mutableStateOf(true) }
    var appName by rememberSaveable { mutableStateOf("Settings") }
    val isError = appName.length > AppMigration.MAX_LABEL_LENGTH || appName.isBlank()

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(CoreR.string.settings_hide_app_title)) },
        text = {
            Column(modifier = Modifier.padding(top = 8.dp)) {
                OutlinedTextField(
                    value = appName,
                    onValueChange = { appName = it },
                    modifier = Modifier.fillMaxWidth(),
                    label = { Text(stringResource(CoreR.string.settings_app_name_hint)) },
                    isError = isError
                )
            }
        },
        confirmButton = {
            TextButton(
                onClick = { if (!isError) onConfirm(appName) },
                enabled = !isError
            ) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

@Composable
private fun RestoreAppDialog(onDismiss: () -> Unit, onConfirm: () -> Unit) {
    val showState = rememberSaveable { mutableStateOf(true) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(CoreR.string.settings_restore_app_title)) },
        text = {
            Text(
                text = stringResource(CoreR.string.restore_app_confirmation),
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurface,
            )
        },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}
