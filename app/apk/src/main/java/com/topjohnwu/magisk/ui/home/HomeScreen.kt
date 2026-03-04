package com.topjohnwu.magisk.ui.home

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.PowerManager
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import android.widget.Toast
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.WindowInsetsSides
import androidx.compose.foundation.layout.only
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.windowInsetsPadding
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
import androidx.lifecycle.lifecycleScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.DpSize
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.getSystemService
import androidx.core.net.toUri
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.LoadingDialogHandle
import com.topjohnwu.magisk.ui.component.MarkdownTextAsync
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.rememberLoadingDialog
import com.topjohnwu.magisk.ui.component.ListPopupDefaults.MenuPositionProvider
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.theme.ThemeState
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR
import top.yukonga.miuix.kmp.basic.ButtonDefaults
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.HorizontalDivider
import top.yukonga.miuix.kmp.basic.VerticalDivider
import top.yukonga.miuix.kmp.basic.Checkbox
import top.yukonga.miuix.kmp.basic.DropdownImpl
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.LinearProgressIndicator
import top.yukonga.miuix.kmp.basic.ListPopupColumn
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.PopupPositionProvider
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.SmallTitle
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.extra.SuperArrow
import top.yukonga.miuix.kmp.extra.SuperBottomSheet
import top.yukonga.miuix.kmp.extra.SuperListPopup
import top.yukonga.miuix.kmp.theme.MiuixTheme
import top.yukonga.miuix.kmp.theme.MiuixTheme.isDynamicColor
import androidx.compose.foundation.background
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.ui.draw.clipToBounds
import androidx.compose.ui.graphics.Color
import top.yukonga.miuix.kmp.basic.CardDefaults
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Close
import top.yukonga.miuix.kmp.icon.extended.Delete
import top.yukonga.miuix.kmp.icon.extended.Hide
import top.yukonga.miuix.kmp.icon.extended.Info
import top.yukonga.miuix.kmp.icon.extended.Ok
import top.yukonga.miuix.kmp.icon.extended.Show

@Composable
fun HomeScreen(viewModel: HomeViewModel, installVm: InstallViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val installUiState by installVm.uiState.collectAsState()
    val context = LocalContext.current
    val activity = context as MainActivity
    val scrollBehavior = MiuixScrollBehavior()
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
                title = stringResource(CoreR.string.section_home),
                scrollBehavior = scrollBehavior,
                actions = {
                    if (Info.isRooted) {
                        RebootButton()
                    }
                }
            )
        },
        popupHost = { }
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

            Row(
                modifier = Modifier.height(IntrinsicSize.Max),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                CoreCard(
                    modifier = Modifier.weight(1f).fillMaxHeight(),
                    state = viewModel.magiskState,
                    version = viewModel.magiskInstalledVersion,
                    remoteVersion = if (viewModel.magiskState == HomeViewModel.State.OUTDATED)
                        "${BuildConfig.APP_VERSION_NAME} (${BuildConfig.APP_VERSION_CODE})" else null,
                    onInstallClicked = { showInstallSheet.value = true },
                    onUninstallClicked = { viewModel.onDeletePressed() },
                )
                AppCard(
                    modifier = Modifier.weight(1f).fillMaxHeight(),
                    state = uiState.appState,
                    version = viewModel.managerInstalledVersion,
                    remoteVersion = if (uiState.appState == HomeViewModel.State.OUTDATED)
                        uiState.managerRemoteVersion else null,
                    progress = uiState.managerProgress,
                    isHidden = context.packageName != BuildConfig.APP_PACKAGE_NAME,
                    onManagerPressed = { viewModel.onManagerPressed() },
                    onHideRestorePressed = viewModel::onHideRestorePressed,
                )
            }

            SmallTitle(text = stringResource(CoreR.string.home_status_title))
            StatusCard()

            val showDonateSheet = rememberSaveable { mutableStateOf(false) }

            SmallTitle(text = stringResource(CoreR.string.home_support_title))
            Card(modifier = Modifier.fillMaxWidth()) {
                SuperArrow(
                    title = stringResource(CoreR.string.documents),
                    onClick = { openLink(context, "https://topjohnwu.github.io/Magisk/") }
                )
                SuperArrow(
                    title = stringResource(CoreR.string.report_bugs),
                    onClick = { openLink(context, "${Const.Url.SOURCE_CODE_URL}/issues") }
                )
                SuperArrow(
                    title = stringResource(CoreR.string.donate),
                    onClick = { showDonateSheet.value = true }
                )
            }

            SupportBottomSheet(
                show = showDonateSheet,
                onLinkClicked = { viewModel.onLinkPressed(it) }
            )

            SmallTitle(text = stringResource(CoreR.string.home_follow_title))
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
            holdDownState = showMenu.value,
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_restart),
                contentDescription = stringResource(CoreR.string.reboot),
            )
        }
        SuperListPopup(
            show = showMenu,
            popupPositionProvider = MenuPositionProvider,
            alignment = PopupPositionProvider.Align.End,
            onDismissRequest = { showMenu.value = false }
        ) {
            ListPopupColumn {
                items.forEachIndexed { index, item ->
                    val isSafeMode = item.labelRes == CoreR.string.reboot_safe_mode
                    DropdownImpl(
                        text = stringResource(item.labelRes),
                        optionSize = items.size,
                        isSelected = isSafeMode && safeModeEnabled >= 2,
                        index = index,
                        onSelectedIndexChange = {
                            item.action()
                            if (!isSafeMode) showMenu.value = false
                        }
                    )
                }
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
                MiuixTheme.colorScheme.tertiaryContainer,
                RoundedCornerShape(16.dp)
            )
            .padding(start = 16.dp, top = 4.dp, bottom = 4.dp, end = 4.dp)
    ) {
        Row(
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(CoreR.string.home_notice_content),
                style = MiuixTheme.textStyles.body2,
                color = MiuixTheme.colorScheme.onTertiaryContainer,
                modifier = Modifier.weight(1f).padding(vertical = 8.dp)
            )
            IconButton(onClick = onHide) {
                Icon(
                    imageVector = MiuixIcons.Close,
                    contentDescription = stringResource(CoreR.string.hide),
                    modifier = Modifier.size(15.dp),
                    tint = MiuixTheme.colorScheme.onTertiaryContainer,
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
    val isDark = when (ThemeState.colorMode) {
        2, 5 -> true
        1, 4 -> false
        else -> isSystemInDarkTheme()
    }
    val cardBg = when (state) {
        HomeViewModel.State.UP_TO_DATE -> when {
            isDynamicColor -> MiuixTheme.colorScheme.secondaryContainer
            isDark -> Color(0xFF1E3026)
            else -> Color(0xFFDFFAE4)
        }
        HomeViewModel.State.OUTDATED -> when {
            isDynamicColor -> MiuixTheme.colorScheme.tertiaryContainer
            isDark -> Color(0xFF302920)
            else -> Color(0xFFFFF3E0)
        }
        else -> Color.Transparent
    }

    val actionLabel = when (state) {
        HomeViewModel.State.OUTDATED -> stringResource(CoreR.string.update)
        HomeViewModel.State.INVALID -> stringResource(CoreR.string.install)
        HomeViewModel.State.UP_TO_DATE -> stringResource(CoreR.string.reinstall)
        HomeViewModel.State.LOADING -> null
    }
    val actionColor = when (state) {
        HomeViewModel.State.OUTDATED, HomeViewModel.State.INVALID -> MiuixTheme.colorScheme.primary
        else -> MiuixTheme.colorScheme.onSurfaceVariantActions
    }
    val uninstallEnabled = Info.env.isActive

    Card(modifier = modifier) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(cardBg)
                .clipToBounds()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Icon(
                    painter = painterResource(CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier.size(24.dp),
                    tint = MiuixTheme.colorScheme.primary
                )
                Spacer(Modifier.height(8.dp))
                Text(
                    text = stringResource(CoreR.string.home_core_title),
                    style = MiuixTheme.textStyles.headline2,
                )
                Text(
                    text = version.ifEmpty { stringResource(CoreR.string.not_available) },
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                )
            }

            Column(
                modifier = Modifier.align(Alignment.TopEnd).padding(4.dp),
                verticalArrangement = Arrangement.spacedBy(0.dp),
            ) {
                IconButton(
                    onClick = onUninstallClicked,
                    enabled = uninstallEnabled,
                ) {
                    Icon(
                        imageVector = MiuixIcons.Delete,
                        contentDescription = null,
                        modifier = Modifier.size(18.dp),
                        tint = if (uninstallEnabled) MiuixTheme.colorScheme.error
                            else MiuixTheme.colorScheme.onSurfaceVariantActions,
                    )
                }
                if (remoteVersion != null) {
                    UpdateBadge(
                        version = remoteVersion,
                        modifier = Modifier.align(Alignment.End).padding(end = 4.dp)
                    )
                }
            }

            if (state != HomeViewModel.State.LOADING) {
                val watermarkIcon = when (state) {
                    HomeViewModel.State.UP_TO_DATE -> MiuixIcons.Ok
                    HomeViewModel.State.OUTDATED -> MiuixIcons.Info
                    else -> MiuixIcons.Close
                }
                val watermarkTint = when (state) {
                    HomeViewModel.State.UP_TO_DATE -> when {
                        isDynamicColor -> MiuixTheme.colorScheme.primary
                        isDark -> Color(0xFF4CAF50)
                        else -> Color(0xFF66BB6A)
                    }
                    HomeViewModel.State.OUTDATED -> when {
                        isDynamicColor -> MiuixTheme.colorScheme.onTertiaryContainer
                        isDark -> Color(0xFFFF9800)
                        else -> Color(0xFFFFA726)
                    }
                    else -> MiuixTheme.colorScheme.onSurfaceVariantSummary
                }
                Icon(
                    imageVector = watermarkIcon,
                    contentDescription = null,
                    modifier = Modifier
                        .matchParentSize()
                        .wrapContentSize(Alignment.TopEnd, unbounded = true)
                        .size(128.dp)
                        .offset(x = 24.dp, y = (-12).dp),
                    tint = watermarkTint.copy(alpha = 0.15f)
                )
            }
        }

        if (actionLabel != null) {
            HorizontalDivider(thickness = 0.75.dp)
            Text(
                text = actionLabel,
                style = MiuixTheme.textStyles.body2,
                color = actionColor,
                textAlign = TextAlign.Center,
                maxLines = 1,
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable(onClick = onInstallClicked)
                    .padding(horizontal = 12.dp, vertical = 12.dp)
            )
        }
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
    val actionColor = when (state) {
        HomeViewModel.State.OUTDATED -> MiuixTheme.colorScheme.primary
        else -> MiuixTheme.colorScheme.onSurfaceVariantActions
    }
    val hideRestoreIcon = if (isHidden) MiuixIcons.Show else MiuixIcons.Hide

    Card(modifier = modifier) {
        Box(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(16.dp)) {
                Icon(
                    painter = painterResource(R.drawable.ic_manager),
                    contentDescription = null,
                    modifier = Modifier.size(24.dp),
                    tint = MiuixTheme.colorScheme.primary
                )
                Spacer(Modifier.height(8.dp))
                Text(
                    text = stringResource(CoreR.string.home_app_title),
                    style = MiuixTheme.textStyles.headline2,
                )
                Text(
                    text = version,
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                )
                if (progress in 1..99) {
                    Spacer(Modifier.height(8.dp))
                    LinearProgressIndicator(
                        progress = progress / 100f,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }

            Column(
                modifier = Modifier.align(Alignment.TopEnd).padding(4.dp),
                verticalArrangement = Arrangement.spacedBy(0.dp),
            ) {
                if (Info.env.isActive) {
                    IconButton(onClick = onHideRestorePressed) {
                        Icon(
                            imageVector = hideRestoreIcon,
                            contentDescription = null,
                            modifier = Modifier.size(18.dp),
                            tint = MiuixTheme.colorScheme.primary,
                        )
                    }
                }
                if (remoteVersion != null) {
                    UpdateBadge(
                        version = remoteVersion,
                        modifier = Modifier.align(Alignment.End).padding(end = 4.dp)
                    )
                }
            }
        }

        if (actionLabel != null) {
            HorizontalDivider(thickness = 0.75.dp)
            Text(
                text = actionLabel,
                style = MiuixTheme.textStyles.body2,
                color = actionColor,
                textAlign = TextAlign.Center,
                maxLines = 1,
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable(onClick = onManagerPressed)
                    .padding(horizontal = 12.dp, vertical = 12.dp)
            )
        }
    }
}

@Composable
private fun UpdateBadge(version: String, modifier: Modifier = Modifier) {
    Text(
        text = version,
        color = MiuixTheme.colorScheme.onPrimary,
        fontSize = 10.sp,
        maxLines = 1,
        modifier = modifier
            .background(MiuixTheme.colorScheme.primary, RoundedCornerShape(6.dp))
            .padding(horizontal = 6.dp, vertical = 2.dp)
    )
}

@Composable
private fun StatusCard() {
    Card(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier.fillMaxWidth().height(IntrinsicSize.Min),
        ) {
            Column(
                modifier = Modifier.weight(1f).padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
            ) {
                Text(
                    text = stringResource(CoreR.string.ramdisk),
                    style = MiuixTheme.textStyles.headline2,
                )
                Text(
                    text = stringResource(if (Info.ramdisk) CoreR.string.yes else CoreR.string.no),
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                )
            }
            VerticalDivider(thickness = 0.75.dp)
            Column(
                modifier = Modifier.weight(1f).padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
            ) {
                Text(
                    text = stringResource(CoreR.string.zygisk),
                    style = MiuixTheme.textStyles.headline2,
                )
                Text(
                    text = stringResource(if (Info.isZygiskEnabled) CoreR.string.yes else CoreR.string.no),
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                )
            }
            VerticalDivider(thickness = 0.75.dp)
            Column(
                modifier = Modifier.weight(1f).padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
            ) {
                Text(
                    text = stringResource(CoreR.string.denylist),
                    style = MiuixTheme.textStyles.headline2,
                )
                Text(
                    text = stringResource(if (Config.denyList) CoreR.string.enabled else CoreR.string.disabled),
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                )
            }
        }
    }
}

@Composable
private fun SupportBottomSheet(
    show: MutableState<Boolean>,
    onLinkClicked: (String) -> Unit,
) {
    SuperBottomSheet(
        show = show,
        onDismissRequest = { show.value = false },
        title = stringResource(CoreR.string.home_support_title),
    ) {
        Column(modifier = Modifier.padding(bottom = 16.dp)) {
            Text(
                text = stringResource(CoreR.string.home_support_content),
                style = MiuixTheme.textStyles.body2,
                color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
            )
            SuperArrow(
                title = stringResource(CoreR.string.patreon),
                onClick = {
                    show.value = false
                    onLinkClicked(Const.Url.PATREON_URL)
                },
                startAction = {
                    Icon(
                        painter = painterResource(CoreR.drawable.ic_patreon),
                        contentDescription = null,
                        modifier = Modifier.size(24.dp),
                        tint = MiuixTheme.colorScheme.onSurfaceVariantActions
                    )
                }
            )
            SuperArrow(
                title = stringResource(CoreR.string.paypal),
                onClick = {
                    show.value = false
                    onLinkClicked("https://paypal.me/magiskdonate")
                },
                startAction = {
                    Icon(
                        painter = painterResource(CoreR.drawable.ic_paypal),
                        contentDescription = null,
                        modifier = Modifier.size(24.dp),
                        tint = MiuixTheme.colorScheme.onSurfaceVariantActions
                    )
                }
            )
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
        LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/shanasaimoe"),
        LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/yujincheng08"),
        LinkInfo("Sponsor", CoreR.drawable.ic_favorite, "https://github.com/sponsors/yujincheng08"),
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
    var selectedDev by remember { mutableStateOf<DeveloperInfo?>(null) }
    val showSheet = rememberSaveable { mutableStateOf(false) }

    Card(modifier = Modifier.fillMaxWidth()) {
        developers.forEach { dev ->
            SuperArrow(
                title = "@${dev.name}",
                onClick = {
                    selectedDev = dev
                    showSheet.value = true
                }
            )
        }
    }

    val currentDev = selectedDev
    if (currentDev != null) {
        SuperBottomSheet(
            show = showSheet,
            onDismissRequest = {
                showSheet.value = false
                selectedDev = null
            },
            title = "@${currentDev.name}",
        ) {
            Column(modifier = Modifier.padding(bottom = 16.dp)) {
                currentDev.links.forEach { link ->
                    SuperArrow(
                        title = link.label,
                        onClick = {
                            showSheet.value = false
                            onLinkClicked(link.url)
                            selectedDev = null
                        },
                        startAction = {
                            Icon(
                                painter = painterResource(link.icon),
                                contentDescription = null,
                                modifier = Modifier.size(24.dp),
                                tint = MiuixTheme.colorScheme.onSurfaceVariantActions
                            )
                        }
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

@Composable
private fun InstallBottomSheet(
    show: MutableState<Boolean>,
    installVm: InstallViewModel,
    installUiState: InstallViewModel.UiState,
) {
    SuperBottomSheet(
        show = show,
        onDismissRequest = { show.value = false },
        title = stringResource(CoreR.string.install),
    ) {
        Column(modifier = Modifier.padding(bottom = 16.dp)) {
            if (installUiState.notes.isNotEmpty()) {
                Text(
                    text = installUiState.notes,
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                    modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
                )
                HorizontalDivider(thickness = 0.75.dp)
            }

            if (!installVm.skipOptions) {
                InstallOptionsSection(installUiState, installVm)
            }

            SuperArrow(
                title = stringResource(CoreR.string.select_patch_file),
                summary = stringResource(CoreR.string.select_patch_file_summary),
                onClick = {
                    show.value = false
                    installVm.selectMethod(InstallViewModel.Method.PATCH)
                },
                enabled = installUiState.step >= 1 || installVm.skipOptions
            )

            if (installVm.isRooted) {
                SuperArrow(
                    title = stringResource(CoreR.string.direct_install),
                    summary = stringResource(CoreR.string.direct_install_summary),
                    onClick = {
                        show.value = false
                        installVm.selectMethod(InstallViewModel.Method.DIRECT)
                        installVm.install()
                    },
                    enabled = installUiState.step >= 1 || installVm.skipOptions
                )
            }

            if (!installVm.noSecondSlot) {
                SuperArrow(
                    title = stringResource(CoreR.string.install_inactive_slot),
                    summary = stringResource(CoreR.string.install_inactive_slot_summary),
                    onClick = {
                        show.value = false
                        installVm.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                    },
                    enabled = installUiState.step >= 1 || installVm.skipOptions
                )
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
                style = MiuixTheme.textStyles.headline2,
            )
            if (uiState.step == 0) {
                TextButton(
                    text = stringResource(CoreR.string.install_next),
                    onClick = { viewModel.nextStep() }
                )
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
            style = MiuixTheme.textStyles.body1,
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
    top.yukonga.miuix.kmp.extra.SuperDialog(
        show = showDialog,
        title = stringResource(CoreR.string.uninstall_magisk_title),
        onDismissRequest = { showDialog.value = false },
    ) {
        Text(
            text = stringResource(CoreR.string.uninstall_magisk_msg),
            style = MiuixTheme.textStyles.body1,
            color = MiuixTheme.colorScheme.onSurface,
        )
        Spacer(Modifier.height(16.dp))
        Row(modifier = Modifier.fillMaxWidth()) {
            TextButton(
                text = stringResource(CoreR.string.restore_img),
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
                },
                modifier = Modifier.weight(1f)
            )
            Spacer(Modifier.width(20.dp))
            TextButton(
                text = stringResource(CoreR.string.complete_uninstall),
                onClick = {
                    showDialog.value = false
                    val intent = Intent(activity, activity.javaClass).apply {
                        action = FlashUtils.INTENT_FLASH
                        putExtra(FlashUtils.EXTRA_FLASH_ACTION, Const.Value.UNINSTALL)
                        flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP
                    }
                    activity.startActivity(intent)
                },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.textButtonColorsPrimary()
            )
        }
    }
}

@Composable
private fun ManagerInstallComposableDialog(
    showDialog: MutableState<Boolean>,
    activity: MainActivity,
) {
    top.yukonga.miuix.kmp.extra.SuperDialog(
        show = showDialog,
        title = stringResource(CoreR.string.install),
        onDismissRequest = { showDialog.value = false },
    ) {
        MarkdownTextAsync {
            val text = Info.update.note
            java.io.File(activity.cacheDir, "${Info.update.versionCode}.md").writeText(text)
            text
        }
        Spacer(Modifier.height(16.dp))
        Row(modifier = Modifier.fillMaxWidth()) {
            TextButton(
                text = stringResource(android.R.string.cancel),
                onClick = { showDialog.value = false },
                modifier = Modifier.weight(1f)
            )
            Spacer(Modifier.width(20.dp))
            TextButton(
                text = stringResource(CoreR.string.install),
                onClick = {
                    showDialog.value = false
                    DownloadEngine.startWithActivity(activity, activity.extension, Subject.App())
                },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.textButtonColorsPrimary()
            )
        }
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

    top.yukonga.miuix.kmp.extra.SuperDialog(
        show = showDialog,
        title = stringResource(CoreR.string.env_fix_title),
        onDismissRequest = { showDialog.value = false },
    ) {
        Text(
            text = stringResource(
                if (needsFullFix) CoreR.string.env_full_fix_msg else CoreR.string.env_fix_msg
            ),
            style = MiuixTheme.textStyles.body1,
            color = MiuixTheme.colorScheme.onSurface,
        )
        Spacer(Modifier.height(16.dp))
        Row(modifier = Modifier.fillMaxWidth()) {
            TextButton(
                text = stringResource(android.R.string.cancel),
                onClick = { showDialog.value = false },
                modifier = Modifier.weight(1f)
            )
            Spacer(Modifier.width(20.dp))
            TextButton(
                text = stringResource(android.R.string.ok),
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
                },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.textButtonColorsPrimary()
            )
        }
    }
}

@Composable
private fun HideAppDialog(onDismiss: () -> Unit, onConfirm: (String) -> Unit) {
    val showState = rememberSaveable { mutableStateOf(true) }
    var appName by rememberSaveable { mutableStateOf("Settings") }
    val isError = appName.length > AppMigration.MAX_LABEL_LENGTH || appName.isBlank()

    top.yukonga.miuix.kmp.extra.SuperDialog(
        show = showState,
        title = stringResource(CoreR.string.settings_hide_app_title),
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            top.yukonga.miuix.kmp.basic.TextField(
                value = appName,
                onValueChange = { appName = it },
                modifier = Modifier.fillMaxWidth(),
                label = stringResource(CoreR.string.settings_app_name_hint),
            )
            Spacer(Modifier.height(16.dp))
            Row(horizontalArrangement = Arrangement.SpaceBetween) {
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
private fun RestoreAppDialog(onDismiss: () -> Unit, onConfirm: () -> Unit) {
    val showState = rememberSaveable { mutableStateOf(true) }

    top.yukonga.miuix.kmp.extra.SuperDialog(
        show = showState,
        title = stringResource(CoreR.string.settings_restore_app_title),
        onDismissRequest = onDismiss,
        insideMargin = DpSize(24.dp, 24.dp)
    ) {
        Column(modifier = Modifier.padding(top = 8.dp)) {
            Text(
                text = stringResource(CoreR.string.restore_app_confirmation),
                style = MiuixTheme.textStyles.body1,
                color = MiuixTheme.colorScheme.onSurface,
            )
            Spacer(Modifier.height(16.dp))
            Row(horizontalArrangement = Arrangement.SpaceBetween) {
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
