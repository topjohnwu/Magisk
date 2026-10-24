package com.topjohnwu.magisk.ui.home

import android.os.Build
import android.os.PowerManager
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.consumeWindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Android
import androidx.compose.material.icons.filled.Build
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.Memory
import androidx.compose.material.icons.filled.PowerSettingsNew
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.RestartAlt
import androidx.compose.material.icons.filled.Security
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.VerticalDivider
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import androidx.core.content.getSystemService
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.compose.dropUnlessResumed
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.component.AdaptiveColumns
import com.topjohnwu.magisk.ui.component.LoadingDialog
import com.topjohnwu.magisk.ui.component.MagiskDialog
import com.topjohnwu.magisk.ui.component.MarkdownTextAsync
import com.topjohnwu.magisk.ui.component.onlyHorizontal
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.ui.install.InstallDialog
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Route
import java.io.File
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    viewModel: HomeViewModel,
    installVm: InstallViewModel,
    modifier: Modifier = Modifier
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val context = LocalContext.current
    val navigator = LocalNavigator.current
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()

    var showUninstallDialog by rememberSaveable { mutableStateOf(false) }
    // Keep the update itself, so a restored dialog doesn't depend on Info.update, which is reset
    // on connectivity changes and after process death
    var managerUpdate by rememberSaveable { mutableStateOf<UpdateInfo?>(null) }
    var showEnvFixDialog by rememberSaveable { mutableStateOf(false) }
    var showInstallDialog by rememberSaveable { mutableStateOf(false) }
    var envFixCode by rememberSaveable { mutableIntStateOf(0) }

    LaunchedEffect(uiState.showUninstall) {
        if (uiState.showUninstall) {
            showUninstallDialog = true
            viewModel.onUninstallConsumed()
        }
    }
    LaunchedEffect(uiState.showManagerInstall) {
        uiState.showManagerInstall?.let {
            managerUpdate = it
            viewModel.onManagerInstallConsumed()
        }
    }
    LaunchedEffect(uiState.envFixCode) {
        if (uiState.envFixCode != 0) {
            envFixCode = uiState.envFixCode
            showEnvFixDialog = true
            viewModel.onEnvFixConsumed()
        }
    }

    LoadingDialog(visible = uiState.showLoading)

    if (showUninstallDialog) {
        UninstallComposableDialog(
            onDismiss = { showUninstallDialog = false },
            // Drop taps while a screen transition runs, so a quick second tap can't push again
            onCompleteUninstall = dropUnlessResumed {
                showUninstallDialog = false
                navigator.push(Route.Flash(Const.Value.UNINSTALL))
            },
            onRestoreImage = {
                showUninstallDialog = false
                viewModel.restoreImages()
            }
        )
    }

    managerUpdate?.let { update ->
        ManagerInstallComposableDialog(
            update = update,
            cacheDir = context.cacheDir,
            onDismiss = { managerUpdate = null },
            onInstall = {
                managerUpdate = null
                (context as? MainActivity)?.let {
                    DownloadEngine.startWithActivity(it, Subject.App(update))
                }
            }
        )
    }

    if (showEnvFixDialog) {
        EnvFixComposableDialog(
            code = envFixCode,
            onDismiss = { showEnvFixDialog = false },
            onNavigateInstall = {
                showEnvFixDialog = false
                showInstallDialog = true
            },
            onFixEnv = {
                showEnvFixDialog = false
                viewModel.fixEnv()
            }
        )
    }

    val scrollState = rememberScrollState()

    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.section_home)) },
                scrollBehavior = scrollBehavior,
                actions = {
                    if (Info.env.isActive) {
                        IconButton(onClick = { viewModel.onDeletePressed() }) {
                            Icon(
                                imageVector = Icons.Default.Delete,
                                contentDescription = stringResource(CoreR.string.uninstall_magisk_title),
                            )
                        }
                    }
                    if (Info.isRooted) {
                        RebootButton()
                    }
                }
            )
        }
    ) { padding ->
        AdaptiveColumns(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .consumeWindowInsets(padding)
                .padding(padding.onlyHorizontal())
                .verticalScrollbar(
                    scrollState,
                    contentPadding = PaddingValues(
                        top = padding.calculateTopPadding() + 12.dp,
                        bottom = padding.calculateBottomPadding() + 12.dp,
                    ),
                )
                .verticalScroll(scrollState)
                .padding(
                    start = 16.dp,
                    top = padding.calculateTopPadding() + 12.dp,
                    end = 16.dp,
                    bottom = padding.calculateBottomPadding() + 12.dp,
                ),
            spacing = 12.dp,
            first = {
                if (uiState.isNoticeVisible) {
                    NoticeCard(onHide = viewModel::hideNotice)
                }

                CoreCard(
                    modifier = Modifier.fillMaxWidth(),
                    state = uiState.magiskState,
                    version = uiState.magiskInstalledVersion,
                    onInstallClicked = { showInstallDialog = true }
                )

                StatusCard()

                AppCard(
                    modifier = Modifier.fillMaxWidth(),
                    state = uiState.appState,
                    version = uiState.managerInstalledVersion,
                    remoteVersion = uiState.managerRemoteVersion,
                    progress = uiState.managerProgress,
                    onManagerPressed = viewModel::onManagerPressed,
                )
            },
            second = {
                Text(
                    text = stringResource(CoreR.string.home_support_title),
                    style = MaterialTheme.typography.labelLarge,
                    fontWeight = FontWeight.SemiBold,
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.padding(start = 16.dp, top = 8.dp, bottom = 4.dp)
                )

                SupportCard(onLinkClicked = viewModel::onLinkPressed)

                Text(
                    text = stringResource(CoreR.string.home_follow_title),
                    style = MaterialTheme.typography.labelLarge,
                    fontWeight = FontWeight.SemiBold,
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.padding(start = 16.dp, top = 8.dp, bottom = 4.dp)
                )
                DevelopersCard(onLinkClicked = viewModel::onLinkPressed)
            },
        )
    }

    InstallDialog(
        show = showInstallDialog,
        onDismiss = { showInstallDialog = false },
        installVm = installVm,
    )
}

@Composable
private fun RebootButton(
    modifier: Modifier = Modifier
) {
    var showMenu by remember { mutableStateOf(false) }
    val context = LocalContext.current
    var safeModeEnabled by remember { mutableIntStateOf(Config.bootloop) }

    val showUserspace = Build.VERSION.SDK_INT >= Build.VERSION_CODES.R &&
        context.getSystemService<PowerManager>()?.isRebootingUserspaceSupported == true
    val showSafeMode = Const.Version.atLeast_28_0()

    val items = buildList {
        add(RebootOption(CoreR.string.reboot, Icons.Default.RestartAlt) { reboot() })
        if (showUserspace) {
            add(RebootOption(CoreR.string.reboot_userspace, Icons.Default.Refresh) { reboot("userspace") })
        }
        add(RebootOption(CoreR.string.reboot_recovery, Icons.Default.Build) { reboot("recovery") })
        add(RebootOption(CoreR.string.reboot_bootloader, Icons.Default.Android) { reboot("bootloader") })
        add(RebootOption(CoreR.string.reboot_download, Icons.Default.Download) { reboot("download") })
        add(RebootOption(CoreR.string.reboot_edl, Icons.Default.Memory) { reboot("edl") })
        if (showSafeMode) {
            add(RebootOption(CoreR.string.reboot_safe_mode, Icons.Default.Security) {
                val newVal = if (safeModeEnabled >= 2) 0 else 2
                Config.bootloop = newVal
                safeModeEnabled = newVal
            })
        }
    }

    Box(modifier = modifier) {
        IconButton(
            onClick = { showMenu = true },
        ) {
            Icon(
                imageVector = Icons.Default.PowerSettingsNew,
                contentDescription = stringResource(CoreR.string.reboot),
            )
        }
        DropdownMenu(
            expanded = showMenu,
            onDismissRequest = { showMenu = false },
            offset = DpOffset(x = (-8).dp, y = 0.dp),
            shape = RoundedCornerShape(16.dp)
        ) {
            items.forEach { item ->
                val isSafeMode = item.labelRes == CoreR.string.reboot_safe_mode
                if (isSafeMode) {
                    HorizontalDivider(
                        modifier = Modifier.padding(vertical = 4.dp),
                        color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f)
                    )
                }
                DropdownMenuItem(
                    text = {
                        Text(
                            text = stringResource(item.labelRes),
                            style = MaterialTheme.typography.bodyLarge
                        )
                    },
                    leadingIcon = {
                        Icon(
                            imageVector = item.icon,
                            contentDescription = null,
                            modifier = Modifier.size(20.dp)
                        )
                    },
                    trailingIcon = if (isSafeMode && safeModeEnabled >= 2) {
                        {
                            Icon(
                                imageVector = Icons.Default.Check,
                                contentDescription = null,
                                tint = MaterialTheme.colorScheme.primary,
                                modifier = Modifier.size(20.dp)
                            )
                        }
                    } else null,
                    onClick = {
                        item.action()
                        if (!isSafeMode) showMenu = false
                    }
                )
            }
        }
    }
}

private class RebootOption(val labelRes: Int, val icon: ImageVector, val action: () -> Unit)

@Composable
private fun NoticeCard(
    onHide: () -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.tertiaryContainer)
    ) {
        Row(
            modifier = Modifier.padding(start = 16.dp, top = 6.dp, bottom = 6.dp, end = 6.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(CoreR.string.home_notice_content),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onTertiaryContainer,
                modifier = Modifier
                    .weight(1f)
                    .padding(vertical = 6.dp)
            )
            IconButton(onClick = onHide) {
                Icon(
                    imageVector = Icons.Default.Close,
                    contentDescription = stringResource(CoreR.string.hide),
                    modifier = Modifier.size(18.dp),
                    tint = MaterialTheme.colorScheme.onTertiaryContainer,
                )
            }
        }
    }
}

@Composable
private fun InstallButton(
    label: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    isPrimary: Boolean = false,
) {
    val buttonContent = @Composable {
        Icon(
            painter = painterResource(R.drawable.ic_download),
            contentDescription = null,
            modifier = Modifier.size(ButtonDefaults.IconSize),
        )
        Spacer(Modifier.width(ButtonDefaults.IconSpacing))
        Text(
            text = label,
            style = MaterialTheme.typography.labelLarge,
        )
    }

    if (isPrimary) {
        Button(
            onClick = onClick,
            contentPadding = ButtonDefaults.ButtonWithIconContentPadding,
            modifier = modifier,
        ) {
            buttonContent()
        }
    } else {
        FilledTonalButton(
            onClick = onClick,
            contentPadding = ButtonDefaults.ButtonWithIconContentPadding,
            modifier = modifier,
        ) {
            buttonContent()
        }
    }
}

@Composable
private fun CoreCard(
    state: HomeViewModel.State,
    version: String,
    onInstallClicked: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val isInstalled = state != HomeViewModel.State.INVALID
    val actionLabel = when (state) {
        HomeViewModel.State.OUTDATED -> stringResource(CoreR.string.update)
        HomeViewModel.State.INVALID -> stringResource(CoreR.string.install)
        HomeViewModel.State.UP_TO_DATE -> stringResource(CoreR.string.reinstall)
        HomeViewModel.State.LOADING -> null
    }

    Card(
        modifier = modifier,
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(18.dp),
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
                    tint = if (isInstalled) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outline,
                    modifier = Modifier.size(48.dp)
                )
                Spacer(Modifier.width(16.dp))
                Column {
                    Text(
                        text = stringResource(CoreR.string.magisk),
                        style = MaterialTheme.typography.titleLarge
                    )
                    Text(
                        text = if (isInstalled) {
                            version.ifEmpty { stringResource(CoreR.string.not_available) }
                        } else {
                            stringResource(CoreR.string.not_installed)
                        },
                        style = MaterialTheme.typography.bodyMedium,
                        color = if (isInstalled) {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        } else {
                            MaterialTheme.colorScheme.error
                        }
                    )
                }
            }

            if (actionLabel != null) {
                InstallButton(
                    label = actionLabel,
                    onClick = onInstallClicked,
                    isPrimary = !isInstalled,
                )
            }
        }
    }
}


@Composable
private fun AppCard(
    state: HomeViewModel.State,
    version: String,
    remoteVersion: String,
    progress: Int,
    onManagerPressed: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val actionLabel = when (state) {
        HomeViewModel.State.OUTDATED -> stringResource(CoreR.string.update)
        HomeViewModel.State.UP_TO_DATE -> stringResource(CoreR.string.reinstall)
        else -> null
    }

    Card(
        modifier = modifier,
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(18.dp)
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
                        tint = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.size(48.dp)
                    )
                    Spacer(Modifier.width(16.dp))
                    Text(
                        text = stringResource(CoreR.string.home_app_title),
                        style = MaterialTheme.typography.titleLarge
                    )
                }

                if (actionLabel != null) {
                    InstallButton(
                        label = actionLabel,
                        onClick = onManagerPressed,
                    )
                }
            }

            Spacer(Modifier.height(16.dp))

            if (state != HomeViewModel.State.LOADING) {
                AppDetailRow(label = stringResource(CoreR.string.home_latest_version), value = remoteVersion)
            }
            AppDetailRow(label = stringResource(CoreR.string.home_installed_version), value = version)
            AppDetailRow(label = stringResource(CoreR.string.home_package), value = LocalContext.current.packageName)

            if (progress in 1..99) {
                Spacer(Modifier.height(8.dp))
                LinearProgressIndicator(
                    progress = { progress / 100f },
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}

@Composable
private fun AppDetailRow(
    label: String,
    value: String,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Text(
            text = value,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurface
        )
    }
}

private data class StatusInfo(val label: String, val status: String)

@Composable
private fun StatusCard(
    modifier: Modifier = Modifier
) {
    val statuses = listOf(
        StatusInfo(
            label = stringResource(CoreR.string.zygisk),
            status = stringResource(if (Info.isZygiskEnabled) CoreR.string.enabled else CoreR.string.disabled)
        ),
        StatusInfo(
            label = stringResource(CoreR.string.ramdisk),
            status = stringResource(if (Info.ramdisk) CoreR.string.yes else CoreR.string.no)
        )
    )

    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(IntrinsicSize.Min),
            verticalAlignment = Alignment.CenterVertically
        ) {
            statuses.forEachIndexed { index, info ->
                Column(
                    modifier = Modifier
                        .weight(1f)
                        .padding(16.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    Text(
                        text = info.label,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Spacer(Modifier.height(4.dp))
                    Text(
                        text = info.status,
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                }
                if (index < statuses.lastIndex) {
                    VerticalDivider(
                        thickness = 0.5.dp,
                        modifier = Modifier.padding(vertical = 12.dp),
                        color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f)
                    )
                }
            }
        }
    }
}

@Composable
private fun SupportCard(
    onLinkClicked: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    ) {
        Column(modifier = Modifier.padding(18.dp)) {
            Text(
                text = stringResource(CoreR.string.home_support_content),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(Modifier.height(16.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp, Alignment.CenterHorizontally),
                verticalAlignment = Alignment.CenterVertically
            ) {
                FilledTonalButton(
                    onClick = { onLinkClicked(Const.Url.PATREON_URL) },
                    contentPadding = ButtonDefaults.ButtonWithIconContentPadding
                ) {
                    Icon(
                        painter = painterResource(CoreR.drawable.ic_patreon),
                        contentDescription = null,
                        modifier = Modifier.size(ButtonDefaults.IconSize),
                    )
                    Spacer(Modifier.width(ButtonDefaults.IconSpacing))
                    Text(
                        text = stringResource(CoreR.string.patreon),
                        style = MaterialTheme.typography.labelLarge
                    )
                }
                FilledTonalButton(
                    onClick = { onLinkClicked("https://paypal.me/magiskdonate") },
                    contentPadding = ButtonDefaults.ButtonWithIconContentPadding
                ) {
                    Icon(
                        painter = painterResource(CoreR.drawable.ic_paypal),
                        contentDescription = null,
                        modifier = Modifier.size(ButtonDefaults.IconSize),
                    )
                    Spacer(Modifier.width(ButtonDefaults.IconSpacing))
                    Text(
                        text = stringResource(CoreR.string.paypal),
                        style = MaterialTheme.typography.labelLarge
                    )
                }
            }
        }
    }
}

private data class LinkInfo(val label: Int, val icon: Int, val url: String)
private data class DeveloperInfo(val name: String, val links: List<LinkInfo>)

private val developers = listOf(
    DeveloperInfo("topjohnwu", listOf(
        LinkInfo(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/topjohnwu"),
        LinkInfo(CoreR.string.github, CoreR.drawable.ic_github, Const.Url.SOURCE_CODE_URL),
    )),
    DeveloperInfo("vvb2060", listOf(
        LinkInfo(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/vvb2060"),
        LinkInfo(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/vvb2060"),
    )),
    DeveloperInfo("yujincheng08", listOf(
        LinkInfo(CoreR.string.sponsor, CoreR.drawable.ic_favorite, "https://github.com/sponsors/yujincheng08"),
        LinkInfo(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/shanasaimoe"),
        LinkInfo(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/yujincheng08"),
    )),
    DeveloperInfo("aviraxp", listOf(
        LinkInfo(CoreR.string.sponsor, CoreR.drawable.ic_favorite, "https://ko-fi.com/A46115EM"),
        LinkInfo(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/aviraxp"),
    )),
    DeveloperInfo("canyie", listOf(
        LinkInfo(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/canyie2977"),
        LinkInfo(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/canyie"),
    )),
)

@Composable
private fun DevelopersCard(
    onLinkClicked: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    ) {
        Column {
            developers.forEachIndexed { index, dev ->
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 18.dp, vertical = 10.dp),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = "@${dev.name}",
                        style = MaterialTheme.typography.bodyLarge
                    )
                    Row(
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        dev.links.forEach { link ->
                            IconButton(onClick = { onLinkClicked(link.url) }) {
                                Icon(
                                    painter = painterResource(link.icon),
                                    contentDescription = stringResource(link.label),
                                    modifier = Modifier.size(24.dp)
                                )
                            }
                        }
                    }
                }
                if (index < developers.lastIndex) {
                    HorizontalDivider(
                        thickness = 0.5.dp,
                        modifier = Modifier.padding(horizontal = 18.dp),
                        color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f)
                    )
                }
            }
        }
    }
}

@Composable
private fun UninstallComposableDialog(
    onDismiss: () -> Unit,
    onCompleteUninstall: () -> Unit,
    onRestoreImage: () -> Unit,
    modifier: Modifier = Modifier
) {
    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = stringResource(CoreR.string.uninstall_magisk_title),
        confirmText = stringResource(CoreR.string.complete_uninstall),
        onConfirm = onCompleteUninstall,
        dismissText = stringResource(CoreR.string.restore_img),
        onDismiss = onRestoreImage,
    ) {
        Text(text = stringResource(CoreR.string.uninstall_magisk_msg))
    }
}

@Composable
private fun ManagerInstallComposableDialog(
    update: UpdateInfo,
    cacheDir: File,
    onDismiss: () -> Unit,
    onInstall: () -> Unit,
    modifier: Modifier = Modifier
) {
    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = stringResource(CoreR.string.install),
        confirmText = stringResource(CoreR.string.install),
        onConfirm = onInstall,
        dismissText = stringResource(android.R.string.cancel),
        onDismiss = onDismiss,
        scrollable = true,
    ) {
        MarkdownTextAsync {
            val text = update.note
            File(cacheDir, "${update.versionCode}.md").writeText(text)
            text
        }
    }
}

@Composable
private fun EnvFixComposableDialog(
    code: Int,
    onDismiss: () -> Unit,
    onNavigateInstall: () -> Unit,
    onFixEnv: () -> Unit,
    modifier: Modifier = Modifier
) {
    val needsFullFix = code == 2 ||
        Info.env.versionCode != BuildConfig.APP_VERSION_CODE ||
        Info.env.versionString != BuildConfig.APP_VERSION_NAME

    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = stringResource(CoreR.string.env_fix_title),
        confirmText = stringResource(android.R.string.ok),
        onConfirm = {
            if (needsFullFix) onNavigateInstall() else onFixEnv()
        },
        dismissText = stringResource(android.R.string.cancel),
        onDismiss = onDismiss,
    ) {
        Text(
            text = stringResource(
                if (needsFullFix) CoreR.string.env_full_fix_msg else CoreR.string.env_fix_msg
            )
        )
    }
}
