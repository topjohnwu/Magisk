package com.topjohnwu.magisk.ui.compose.home

import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.content.Intent
import android.net.Uri
import android.widget.TextView
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.compose.animation.*
import androidx.compose.animation.core.spring
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.viewModelScope
import coil.compose.AsyncImage
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.ui.compose.RefreshOnResume
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import retrofit2.http.GET
import retrofit2.http.Headers
import retrofit2.http.Query
import java.util.Locale

@Composable
fun HomeScreen(
    rebootRequestToken: Int = 0,
    onRebootTokenConsumed: () -> Unit = {},
    onOpenInstall: () -> Unit = {},
    onOpenUninstall: () -> Unit = {},
    viewModel: HomeComposeViewModel = viewModel(factory = HomeComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val context = LocalContext.current
    val snackbarHostState = remember { SnackbarHostState() }
    var showUninstallDialog by remember { mutableStateOf(false) }
    var showRebootDialog by remember { mutableStateOf(false) }
    var showManagerInstallSheet by remember { mutableStateOf(false) }

    RefreshOnResume { viewModel.refresh() }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }
    LaunchedEffect(rebootRequestToken) {
        if (rebootRequestToken > 0) {
            onRebootTokenConsumed()
            showRebootDialog = true
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(bottom = 140.dp)
        ) {
            ExpressiveHeader(
                envActive = state.envActive
            )

            Column(
                modifier = Modifier.padding(horizontal = 20.dp),
                verticalArrangement = Arrangement.spacedBy(28.dp)
            ) {
                if (state.noticeVisible) {
                    NoticeCard(onHide = viewModel::hideNotice)
                }

                Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    SectionHeader(
                        stringResource(id = CoreR.string.home_section_magisk_core),
                        Icons.Rounded.VerifiedUser
                    )
                    MagiskOrganicCard(
                        magiskState = state.magiskState,
                        magiskInstalledVersion = state.magiskInstalledVersion,
                        onAction = onOpenInstall
                    )
                }

                Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    SectionHeader(
                        stringResource(id = CoreR.string.home_section_application),
                        Icons.Rounded.AppShortcut
                    )
                    AppOrganicCardXL(
                        appState = state.appState,
                        managerInstalledVersion = state.managerInstalledVersion,
                        managerRemoteVersion = state.managerRemoteVersion,
                        updateChannelName = state.updateChannelName,
                        packageName = state.packageName,
                        onAction = { viewModel.onManagerPressed { showManagerInstallSheet = true } }
                    )
                }

                if (state.envActive) {
                    UninstallAction(onClick = { showUninstallDialog = true })
                }

                Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
                    SectionHeader(
                        stringResource(id = CoreR.string.home_section_contributors),
                        Icons.Rounded.Groups
                    )
                    ContributorsExpressiveList(
                        state.contributors,
                        state.contributorsLoading,
                        onOpen = { viewModel.openLink(context, it) })
                }

                Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
                    SectionHeader(
                        stringResource(id = CoreR.string.home_support_title),
                        Icons.Rounded.Favorite
                    )
                    SupportOrganicSection(
                        onPatreon = { viewModel.openLink(context, Const.Url.PATREON_URL) },
                        onPaypal = { viewModel.openLink(context, Const.Url.PAYPAL_URL) }
                    )
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(bottom = 120.dp)
        )
    }

    if (showRebootDialog) {
        RebootExpressiveDialog(
            onDismiss = { showRebootDialog = false },
            onReboot = { type ->
                showRebootDialog = false
                reboot(type)
            }
        )
    }

    if (showUninstallDialog) {
        UninstallExpressiveDialog(
            onDismiss = { showUninstallDialog = false },
            onRestoreImages = {
                showUninstallDialog = false
                viewModel.restoreImages()
            },
            onCompleteUninstall = {
                showUninstallDialog = false
                onOpenUninstall()
            }
        )
    }

    if (showManagerInstallSheet) {
        ManagerInstallSheet(
            notes = state.managerReleaseNotes,
            onDismiss = { showManagerInstallSheet = false },
            onInstall = {
                showManagerInstallSheet = false
                viewModel.startManagerInstall(context)
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun ManagerInstallSheet(
    notes: String,
    onDismiss: () -> Unit,
    onInstall: () -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        shape = RoundedCornerShape(topStart = 28.dp, topEnd = 28.dp),
        containerColor = MaterialTheme.colorScheme.surfaceContainer,
        dragHandle = { BottomSheetDefaults.DragHandle() }
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .navigationBarsPadding()
                .imePadding()
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Surface(
                    color = MaterialTheme.colorScheme.secondaryContainer,
                    shape = CircleShape,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        Icons.Rounded.SystemUpdateAlt,
                        contentDescription = null,
                        modifier = Modifier.padding(8.dp),
                        tint = MaterialTheme.colorScheme.onSecondaryContainer
                    )
                }
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = stringResource(id = CoreR.string.install),
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Black
                    )
                    Text(
                        text = stringResource(id = CoreR.string.release_notes),
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            Surface(
                shape = RoundedCornerShape(18.dp),
                color = MaterialTheme.colorScheme.surfaceContainerHigh,
                modifier = Modifier
                    .fillMaxWidth()
                    .heightIn(min = 160.dp, max = 420.dp)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .verticalScroll(rememberScrollState())
                        .padding(14.dp)
                ) {
                    HomeMarkdownText(
                        markdown = notes.ifBlank { AppContext.getString(CoreR.string.not_available) },
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                OutlinedButton(
                    onClick = onDismiss,
                    modifier = Modifier
                        .weight(1f)
                        .height(52.dp),
                    shape = RoundedCornerShape(16.dp)
                ) {
                    Text(stringResource(id = android.R.string.cancel), fontWeight = FontWeight.Bold)
                }
                Button(
                    onClick = onInstall,
                    modifier = Modifier
                        .weight(1f)
                        .height(52.dp),
                    shape = RoundedCornerShape(16.dp)
                ) {
                    Icon(Icons.Rounded.DownloadDone, null, modifier = Modifier.size(18.dp))
                    Spacer(Modifier.width(8.dp))
                    Text(stringResource(id = CoreR.string.install), fontWeight = FontWeight.Black)
                }
            }
            Spacer(Modifier.height(8.dp))
        }
    }
}

@Composable
private fun HomeMarkdownText(
    markdown: String,
    modifier: Modifier = Modifier
) {
    val textColor = MaterialTheme.colorScheme.onSurfaceVariant.toArgb()
    AndroidView(
        modifier = modifier,
        factory = { context ->
            TextView(context).apply {
                setTextColor(textColor)
                textSize = 14f
            }
        },
        update = { textView ->
            textView.setTextColor(textColor)
            ServiceLocator.markwon.setMarkdown(textView, markdown)
        }
    )
}

@Composable
private fun ExpressiveHeader(envActive: Boolean) {
    val isInstalled = envActive
    val primaryColor =
        if (isInstalled) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.error
    val containerColor =
        if (isInstalled) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.errorContainer
    val title =
        stringResource(id = if (isInstalled) CoreR.string.home_status_ready else CoreR.string.home_status_inactive)
    val subtitle =
        stringResource(id = if (isInstalled) CoreR.string.home_status_ready_subtitle else CoreR.string.home_status_inactive_subtitle)

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 12.dp)
    ) {
        ElevatedCard(
            modifier = Modifier
                .fillMaxWidth()
                .height(260.dp),
            shape = RoundedCornerShape(
                topStart = 48.dp,
                topEnd = 120.dp,
                bottomStart = 120.dp,
                bottomEnd = 48.dp
            ),
            colors = CardDefaults.elevatedCardColors(containerColor = containerColor),
            elevation = CardDefaults.elevatedCardElevation(defaultElevation = 6.dp)
        ) {
            Box(modifier = Modifier.fillMaxSize()) {
                Icon(
                    painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier
                        .size(240.dp)
                        .align(Alignment.CenterEnd)
                        .offset(x = 60.dp, y = 30.dp)
                        .alpha(0.12f),
                    tint = primaryColor
                )

                Column(
                    modifier = Modifier
                        .align(Alignment.BottomStart)
                        .padding(32.dp)
                ) {
                    Surface(
                        shape = RoundedCornerShape(12.dp),
                        color = primaryColor.copy(alpha = 0.15f),
                    ) {
                        Row(
                            modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            Icon(
                                imageVector = if (isInstalled) Icons.Rounded.Verified else Icons.Rounded.GppMaybe,
                                contentDescription = null,
                                modifier = Modifier.size(16.dp),
                                tint = primaryColor
                            )
                            Text(
                                text = stringResource(
                                    id = if (isInstalled) {
                                        CoreR.string.home_state_up_to_date
                                    } else {
                                        CoreR.string.home_state_inactive
                                    }
                                ),
                                style = MaterialTheme.typography.labelSmall,
                                fontWeight = FontWeight.Black,
                                letterSpacing = 1.sp,
                                color = primaryColor
                            )
                        }
                    }

                    Spacer(Modifier.height(20.dp))

                    Text(
                        text = title,
                        style = MaterialTheme.typography.displaySmall,
                        fontWeight = FontWeight.Black,
                        color = primaryColor,
                        lineHeight = 40.sp
                    )
                    Text(
                        text = subtitle,
                        style = MaterialTheme.typography.titleMedium,
                        color = primaryColor.copy(alpha = 0.7f),
                        fontWeight = FontWeight.Bold
                    )
                }

            }
        }
    }
}

@Composable
private fun MagiskOrganicCard(
    magiskState: HomeViewModel.State,
    magiskInstalledVersion: String,
    onAction: () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(
            topStart = 48.dp,
            bottomEnd = 48.dp,
            topEnd = 16.dp,
            bottomStart = 16.dp
        ),
        colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
    ) {
        Box {
            Icon(
                painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                contentDescription = null,
                modifier = Modifier
                    .size(160.dp)
                    .align(Alignment.TopEnd)
                    .offset(x = 40.dp, y = (-30).dp)
                    .alpha(0.04f),
                tint = MaterialTheme.colorScheme.primary
            )

            Column(modifier = Modifier.padding(28.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Surface(
                        color = MaterialTheme.colorScheme.primaryContainer,
                        shape = RoundedCornerShape(16.dp),
                        modifier = Modifier.size(56.dp)
                    ) {
                        Icon(
                            painter = painterResource(id = CoreR.drawable.ic_magisk),
                            null,
                            modifier = Modifier.padding(14.dp),
                            tint = MaterialTheme.colorScheme.onPrimaryContainer
                        )
                    }
                    Spacer(Modifier.width(20.dp))
                    Column(Modifier.weight(1f)) {
                        Text(
                            stringResource(id = CoreR.string.magisk),
                            style = MaterialTheme.typography.headlineSmall,
                            fontWeight = FontWeight.Black
                        )
                        StatusBadge(magiskState)
                    }
                }
                Spacer(Modifier.height(28.dp))
                BentoInfoGrid(
                    listOf(
                        stringResource(id = CoreR.string.home_installed_version) to magiskInstalledVersion,
                        stringResource(id = CoreR.string.zygisk) to stringResource(id = if (Info.isZygiskEnabled) CoreR.string.yes else CoreR.string.no),
                        stringResource(id = CoreR.string.home_ramdisk) to stringResource(id = if (Info.ramdisk) CoreR.string.yes else CoreR.string.no)
                    )
                )
                Spacer(Modifier.height(28.dp))
                Button(
                    onClick = onAction,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(64.dp),
                    shape = RoundedCornerShape(20.dp)
                ) {
                    Icon(Icons.Rounded.Bolt, null)
                    Spacer(Modifier.width(12.dp))
                    Text(
                        stringResource(
                            id = if (magiskState == HomeViewModel.State.OUTDATED) {
                                CoreR.string.update
                            } else {
                                CoreR.string.install
                            }
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.ExtraBold
                    )
                }
            }
        }
    }
}

@Composable
private fun AppOrganicCardXL(
    appState: HomeViewModel.State,
    managerInstalledVersion: String,
    managerRemoteVersion: String,
    updateChannelName: String,
    packageName: String,
    onAction: () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(
            topStart = 16.dp,
            bottomEnd = 16.dp,
            topEnd = 64.dp,
            bottomStart = 64.dp
        ),
        colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
    ) {
        Column(modifier = Modifier.padding(32.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Surface(
                    color = MaterialTheme.colorScheme.primaryContainer,
                    shape = RoundedCornerShape(20.dp),
                    modifier = Modifier.size(72.dp)
                ) {
                    Icon(
                        Icons.Rounded.AppShortcut,
                        null,
                        modifier = Modifier.padding(18.dp),
                        tint = MaterialTheme.colorScheme.onPrimaryContainer
                    )
                }
                Spacer(Modifier.width(20.dp))
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    Text(
                        stringResource(id = CoreR.string.home_app_title),
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Black,
                        color = MaterialTheme.colorScheme.primary
                    )
                    if (appState != HomeViewModel.State.INVALID && appState != HomeViewModel.State.LOADING) {
                        StatusBadge(appState)
                    }
                }
            }
            Spacer(Modifier.height(32.dp))
            Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    BentoInfoItem(
                        label = stringResource(id = CoreR.string.home_latest_version),
                        value = managerRemoteVersion,
                        modifier = Modifier.weight(1f)
                    )
                    BentoInfoItem(
                        label = stringResource(id = CoreR.string.home_channel),
                        value = updateChannelName,
                        modifier = Modifier.weight(1f)
                    )
                }
                BentoInfoItem(
                    label = stringResource(id = CoreR.string.home_package),
                    value = packageName,
                    modifier = Modifier.fillMaxWidth(),
                    valueMaxLines = 2
                )
            }
            if (appState != HomeViewModel.State.INVALID && appState != HomeViewModel.State.LOADING) {
                Spacer(Modifier.height(32.dp))
                if (appState == HomeViewModel.State.OUTDATED) {
                    Button(
                        onClick = onAction,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(64.dp),
                        shape = RoundedCornerShape(24.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.primary,
                            contentColor = MaterialTheme.colorScheme.onPrimary
                        )
                    ) {
                        Icon(Icons.Rounded.BrowserUpdated, null)
                        Spacer(Modifier.width(12.dp))
                        Text(
                            stringResource(id = CoreR.string.update),
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Black
                        )
                    }
                } else {
                    TextButton(
                        onClick = onAction,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(56.dp),
                        shape = RoundedCornerShape(20.dp),
                        colors = ButtonDefaults.textButtonColors(
                            contentColor = MaterialTheme.colorScheme.primary
                        )
                    ) {
                        Icon(Icons.Rounded.SystemUpdateAlt, null)
                        Spacer(Modifier.width(10.dp))
                        Text(
                            stringResource(id = CoreR.string.install),
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Black
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun BentoInfoGrid(items: List<Pair<String, String>>) {
    if (items.isEmpty()) return

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        val first = items.first()
        BentoInfoItem(
            label = first.first,
            value = first.second,
            modifier = Modifier.fillMaxWidth()
        )

        val remaining = items.drop(1)
        remaining.chunked(2).forEach { rowItems ->
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                rowItems.forEach { (label, value) ->
                    BentoInfoItem(
                        label = label,
                        value = value,
                        modifier = Modifier.weight(1f)
                    )
                }
                if (rowItems.size == 1) {
                    Spacer(modifier = Modifier.weight(1f))
                }
            }
        }
    }
}

@Composable
private fun BentoInfoItem(
    label: String,
    value: String,
    modifier: Modifier = Modifier,
    valueMaxLines: Int = 1
) {
    Column(
        modifier = modifier
            .clip(RoundedCornerShape(20.dp))
            .background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f))
            .padding(16.dp)
    ) {
        Text(
            label,
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            fontWeight = FontWeight.Black
        )
        Spacer(Modifier.height(4.dp))
        Text(
            value,
            style = MaterialTheme.typography.bodySmall,
            fontWeight = FontWeight.ExtraBold,
            maxLines = valueMaxLines,
            overflow = TextOverflow.Ellipsis
        )
    }
}

@Composable
private fun ContributorsExpressiveList(
    contributors: List<Contributor>,
    loading: Boolean,
    onOpen: (String) -> Unit
) {
    if (loading && contributors.isEmpty()) {
        LinearProgressIndicator(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 24.dp)
                .clip(CircleShape),
            strokeCap = StrokeCap.Round
        )
        return
    }

    if (contributors.isEmpty()) return

    val shapes = listOf(
        RoundedCornerShape(
            topStart = 64.dp,
            topEnd = 16.dp,
            bottomStart = 16.dp,
            bottomEnd = 64.dp
        ),
        RoundedCornerShape(
            topStart = 16.dp,
            topEnd = 64.dp,
            bottomStart = 64.dp,
            bottomEnd = 16.dp
        ),
        RoundedCornerShape(
            topStart = 48.dp,
            topEnd = 48.dp,
            bottomStart = 12.dp,
            bottomEnd = 48.dp
        ),
        RoundedCornerShape(topStart = 12.dp, topEnd = 56.dp, bottomStart = 56.dp, bottomEnd = 56.dp)
    )

    val maintainerHandles = MAINTAINER_LINKS.keys
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .horizontalScroll(rememberScrollState())
            .padding(vertical = 8.dp),
        horizontalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        contributors.forEachIndexed { index, user ->
            val shape = shapes[index % shapes.size]
            val isMaintainer = maintainerHandles.contains(user.login.lowercase(Locale.US))
            ElevatedCard(
                modifier = Modifier
                    .width(170.dp)
                    .height(210.dp)
                    .clip(shape)
                    .clickable { onOpen(user.htmlUrl) },
                shape = shape,
                colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(16.dp)
                ) {
                    Column(
                        modifier = Modifier.fillMaxSize(),
                        horizontalAlignment = Alignment.CenterHorizontally,
                        verticalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        Surface(
                            shape = CircleShape,
                            border = androidx.compose.foundation.BorderStroke(
                                2.dp,
                                MaterialTheme.colorScheme.primary.copy(alpha = 0.2f)
                            ),
                            modifier = Modifier.size(72.dp)
                        ) {
                            AsyncImage(
                                model = user.avatarUrl,
                                contentDescription = null,
                                modifier = Modifier.clip(CircleShape),
                                contentScale = ContentScale.Crop
                            )
                        }

                        Column(
                            horizontalAlignment = Alignment.CenterHorizontally,
                            verticalArrangement = Arrangement.spacedBy(4.dp)
                        ) {
                            Text(
                                user.login,
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.Black,
                                textAlign = TextAlign.Center,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                            if (isMaintainer) {
                                Surface(
                                    color = MaterialTheme.colorScheme.primary.copy(alpha = 0.12f),
                                    shape = RoundedCornerShape(8.dp)
                                ) {
                                    Text(
                                        text = stringResource(id = CoreR.string.home_maintainer).uppercase(),
                                        modifier = Modifier.padding(
                                            horizontal = 8.dp,
                                            vertical = 2.dp
                                        ),
                                        style = MaterialTheme.typography.labelSmall,
                                        color = MaterialTheme.colorScheme.primary,
                                        fontWeight = FontWeight.Black
                                    )
                                }
                            }
                        }

                        Spacer(Modifier.weight(1f))

                        Row(
                            horizontalArrangement = Arrangement.spacedBy(10.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            user.links.take(3).forEach { link ->
                                Surface(
                                    modifier = Modifier
                                        .size(34.dp)
                                        .clip(CircleShape)
                                        .clickable { onOpen(link.url) },
                                    color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.5f)
                                ) {
                                    Icon(
                                        painter = painterResource(id = link.iconRes),
                                        contentDescription = stringResource(id = link.labelRes),
                                        modifier = Modifier.padding(9.dp),
                                        tint = MaterialTheme.colorScheme.onPrimaryContainer
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun SupportOrganicSection(onPatreon: () -> Unit, onPaypal: () -> Unit) {
    val patreonAccent = MaterialTheme.colorScheme.tertiary
    val paypalAccent = MaterialTheme.colorScheme.primary

    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(28.dp),
        colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
    ) {
        Column(
            modifier = Modifier.padding(24.dp),
            verticalArrangement = Arrangement.spacedBy(20.dp)
        ) {
            Text(
                text = stringResource(id = CoreR.string.home_support_content),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                lineHeight = 22.sp
            )
            BoxWithConstraints(modifier = Modifier.fillMaxWidth()) {
                val compact = maxWidth < 420.dp
                if (compact) {
                    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                        SupportLinkButton(
                            label = stringResource(id = CoreR.string.patreon),
                            iconRes = CoreR.drawable.ic_patreon,
                            accentColor = patreonAccent,
                            onClick = onPatreon,
                            modifier = Modifier.fillMaxWidth()
                        )
                        SupportLinkButton(
                            label = stringResource(id = CoreR.string.paypal),
                            iconRes = CoreR.drawable.ic_paypal,
                            accentColor = paypalAccent,
                            onClick = onPaypal,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                } else {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        SupportLinkButton(
                            label = stringResource(id = CoreR.string.patreon),
                            iconRes = CoreR.drawable.ic_patreon,
                            accentColor = patreonAccent,
                            onClick = onPatreon,
                            modifier = Modifier.weight(1f)
                        )
                        SupportLinkButton(
                            label = stringResource(id = CoreR.string.paypal),
                            iconRes = CoreR.drawable.ic_paypal,
                            accentColor = paypalAccent,
                            onClick = onPaypal,
                            modifier = Modifier.weight(1f)
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun SupportLinkButton(
    label: String,
    @DrawableRes iconRes: Int,
    accentColor: Color,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    val containerColor = MaterialTheme.colorScheme.surfaceContainerHighest.copy(alpha = 0.85f)
    Button(
        onClick = onClick,
        modifier = modifier.height(58.dp),
        shape = RoundedCornerShape(20.dp),
        colors = ButtonDefaults.buttonColors(
            containerColor = containerColor,
            contentColor = MaterialTheme.colorScheme.onSurface
        ),
        border = BorderStroke(1.dp, accentColor.copy(alpha = 0.28f)),
        elevation = ButtonDefaults.buttonElevation(
            defaultElevation = 0.dp,
            pressedElevation = 0.dp
        ),
        contentPadding = PaddingValues(horizontal = 14.dp, vertical = 8.dp)
    ) {
        Surface(
            color = accentColor.copy(alpha = 0.16f),
            shape = CircleShape,
            modifier = Modifier.size(28.dp)
        ) {
            Icon(
                painter = painterResource(id = iconRes),
                contentDescription = null,
                modifier = Modifier.padding(6.dp),
                tint = accentColor
            )
        }
        Spacer(Modifier.width(8.dp))
        Text(
            text = label.uppercase(Locale.ROOT),
            style = MaterialTheme.typography.labelLarge,
            fontWeight = FontWeight.Bold,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis,
            letterSpacing = 0.4.sp
        )
        Spacer(Modifier.weight(1f))
        Icon(
            imageVector = Icons.Rounded.ArrowOutward,
            contentDescription = null,
            modifier = Modifier.size(16.dp),
            tint = accentColor
        )
    }
}

@Composable
private fun SectionHeader(title: String, icon: ImageVector) {
    Row(
        verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier.padding(horizontal = 4.dp, vertical = 8.dp)
    ) {
        Surface(
            color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f),
            shape = RoundedCornerShape(12.dp),
            modifier = Modifier.size(32.dp)
        ) {
            Icon(
                icon, null,
                modifier = Modifier.padding(6.dp),
                tint = MaterialTheme.colorScheme.onPrimaryContainer
            )
        }
        Spacer(Modifier.width(16.dp))
        Text(
            text = title.uppercase(),
            style = MaterialTheme.typography.labelLarge,
            fontWeight = FontWeight.Black,
            letterSpacing = 1.2.sp,
            color = MaterialTheme.colorScheme.outline
        )
    }
}

@Composable
private fun StatusBadge(state: HomeViewModel.State) {
    val (text, color) = when (state) {
        HomeViewModel.State.UP_TO_DATE -> stringResource(id = CoreR.string.home_state_up_to_date) to Color(
            0xFF4CAF50
        )

        HomeViewModel.State.OUTDATED -> stringResource(id = CoreR.string.home_state_update_ready) to Color(
            0xFFFF9800
        )

        else -> stringResource(id = CoreR.string.home_state_inactive) to MaterialTheme.colorScheme.error
    }
    Surface(color = color.copy(alpha = 0.15f), shape = RoundedCornerShape(8.dp)) {
        Text(
            text = text.uppercase(),
            modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp),
            style = MaterialTheme.typography.labelSmall,
            color = color,
            fontWeight = FontWeight.Black,
            letterSpacing = 0.5.sp
        )
    }
}

@Composable
private fun UninstallAction(onClick: () -> Unit) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(24.dp),
        color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.2f),
        onClick = onClick
    ) {
        Row(modifier = Modifier.padding(24.dp), verticalAlignment = Alignment.CenterVertically) {
            Surface(
                color = MaterialTheme.colorScheme.error,
                shape = CircleShape,
                modifier = Modifier.size(40.dp)
            ) {
                Icon(
                    Icons.Rounded.DeleteForever,
                    null,
                    modifier = Modifier.padding(10.dp),
                    tint = MaterialTheme.colorScheme.onError
                )
            }
            Spacer(Modifier.width(20.dp))
            Text(
                stringResource(id = CoreR.string.home_uninstall_environment),
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Black,
                color = MaterialTheme.colorScheme.error
            )
            Spacer(Modifier.weight(1f))
            Icon(
                Icons.Rounded.ChevronRight,
                null,
                tint = MaterialTheme.colorScheme.error.copy(alpha = 0.5f)
            )
        }
    }
}

@Composable
private fun NoticeCard(onHide: () -> Unit) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(28.dp),
        colors = CardDefaults.elevatedCardColors(
            containerColor = MaterialTheme.colorScheme.tertiaryContainer.copy(
                alpha = 0.7f
            )
        )
    ) {
        Row(modifier = Modifier.padding(20.dp), verticalAlignment = Alignment.CenterVertically) {
            Icon(Icons.Rounded.Info, null, tint = MaterialTheme.colorScheme.onTertiaryContainer)
            Spacer(Modifier.width(16.dp))
            Text(
                stringResource(id = CoreR.string.home_notice_content),
                modifier = Modifier.weight(1f),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onTertiaryContainer,
                lineHeight = 20.sp,
                fontWeight = FontWeight.Medium
            )
            IconButton(onClick = onHide) {
                Icon(
                    Icons.Rounded.Close,
                    null,
                    modifier = Modifier.size(20.dp)
                )
            }
        }
    }
}

@Composable
private fun RebootExpressiveDialog(onDismiss: () -> Unit, onReboot: (String) -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Surface(
                    color = MaterialTheme.colorScheme.primaryContainer,
                    shape = CircleShape,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        Icons.Rounded.RestartAlt,
                        null,
                        modifier = Modifier.padding(8.dp),
                        tint = MaterialTheme.colorScheme.onPrimaryContainer
                    )
                }
                Spacer(Modifier.width(16.dp))
                Text(stringResource(id = CoreR.string.reboot), fontWeight = FontWeight.Black)
            }
        },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                ExpressiveOptionCard(
                    Icons.Rounded.PowerSettingsNew,
                    stringResource(id = CoreR.string.reboot),
                    subtitle = null,
                    accentColor = MaterialTheme.colorScheme.primary
                ) { onReboot("") }
                ExpressiveOptionCard(
                    Icons.Rounded.History,
                    stringResource(id = CoreR.string.reboot_recovery),
                    subtitle = null,
                    accentColor = MaterialTheme.colorScheme.secondary
                ) { onReboot("recovery") }
                ExpressiveOptionCard(
                    Icons.Rounded.Terminal,
                    stringResource(id = CoreR.string.reboot_bootloader),
                    subtitle = null,
                    accentColor = MaterialTheme.colorScheme.tertiary
                ) { onReboot("bootloader") }
                ExpressiveOptionCard(
                    Icons.Rounded.Download,
                    stringResource(id = CoreR.string.reboot_download),
                    subtitle = null,
                    accentColor = MaterialTheme.colorScheme.inversePrimary
                ) { onReboot("download") }
                ExpressiveOptionCard(
                    Icons.Rounded.Bolt,
                    stringResource(id = CoreR.string.reboot_userspace),
                    subtitle = null,
                    accentColor = MaterialTheme.colorScheme.onSurfaceVariant
                ) { onReboot("userspace") }
            }
        },
        confirmButton = {
            TextButton(onClick = onDismiss) {
                Text(
                    stringResource(id = CoreR.string.close),
                    fontWeight = FontWeight.Bold
                )
            }
        },
        shape = RoundedCornerShape(32.dp),
        containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
    )
}

@Composable
private fun ExpressiveOptionCard(
    icon: ImageVector,
    title: String,
    subtitle: String?,
    accentColor: Color,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(20.dp))
            .clickable { onClick() },
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
    ) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Surface(
                color = accentColor.copy(alpha = 0.15f),
                shape = CircleShape,
                modifier = Modifier.size(44.dp)
            ) {
                Icon(icon, null, modifier = Modifier.padding(10.dp), tint = accentColor)
            }
            Spacer(Modifier.width(16.dp))
            Column {
                Text(
                    title,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.ExtraBold
                )
                if (subtitle != null) {
                    Text(
                        subtitle,
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

@Composable
private fun UninstallExpressiveDialog(
    onDismiss: () -> Unit,
    onRestoreImages: () -> Unit,
    onCompleteUninstall: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Surface(
                    color = MaterialTheme.colorScheme.errorContainer,
                    shape = CircleShape,
                    modifier = Modifier.size(36.dp)
                ) {
                    Icon(
                        Icons.Rounded.DeleteSweep,
                        null,
                        modifier = Modifier.padding(8.dp),
                        tint = MaterialTheme.colorScheme.onErrorContainer
                    )
                }
                Spacer(Modifier.width(16.dp))
                Text(
                    stringResource(id = CoreR.string.uninstall_magisk_title),
                    fontWeight = FontWeight.Black
                )
            }
        },
        text = { Text(stringResource(id = CoreR.string.uninstall_magisk_msg)) },
        confirmButton = {
            Column(
                verticalArrangement = Arrangement.spacedBy(12.dp),
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(top = 16.dp)
            ) {
                ExpressiveOptionCard(
                    icon = Icons.Rounded.SettingsBackupRestore,
                    title = stringResource(id = CoreR.string.restore_img),
                    subtitle = stringResource(id = CoreR.string.uninstall_restore_images_subtitle),
                    accentColor = MaterialTheme.colorScheme.primary
                ) { onRestoreImages() }
                ExpressiveOptionCard(
                    icon = Icons.Rounded.DeleteForever,
                    title = stringResource(id = CoreR.string.complete_uninstall),
                    subtitle = stringResource(id = CoreR.string.uninstall_complete_subtitle),
                    accentColor = MaterialTheme.colorScheme.error
                ) { onCompleteUninstall() }
            }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text(stringResource(id = CoreR.string.close)) } },
        shape = RoundedCornerShape(32.dp),
        containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
    )
}

// Logic components
data class ContributorLink(
    @StringRes val labelRes: Int,
    @DrawableRes val iconRes: Int,
    val url: String
)

data class Contributor(
    val login: String,
    val avatarUrl: String,
    val htmlUrl: String,
    val links: List<ContributorLink> = emptyList()
)

private val MAINTAINER_LINKS: Map<String, List<ContributorLink>> = mapOf(
    "topjohnwu" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/topjohnwu"),
        ContributorLink(
            CoreR.string.github,
            CoreR.drawable.ic_github,
            "https://github.com/topjohnwu/Magisk"
        )
    ),
    "vvb2060" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/vvb2060"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/vvb2060")
    ),
    "yujincheng08" to listOf(
        ContributorLink(
            CoreR.string.twitter,
            CoreR.drawable.ic_twitter,
            "https://x.com/yujincheng08"
        ),
        ContributorLink(
            CoreR.string.github,
            CoreR.drawable.ic_github,
            "https://github.com/yujincheng08"
        ),
        ContributorLink(
            CoreR.string.github,
            CoreR.drawable.ic_favorite,
            "https://github.com/sponsors/yujincheng08"
        )
    ),
    "rikkaw" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/rikkaw_"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/RikkaW")
    ),
    "canyie" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/canyieq"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/canyie")
    )
)

private fun createContributor(login: String, avatarUrl: String, htmlUrl: String): Contributor {
    val normalized = login.lowercase(Locale.US)
    return Contributor(
        login = login,
        avatarUrl = avatarUrl,
        htmlUrl = htmlUrl,
        links = MAINTAINER_LINKS[normalized].orEmpty()
    )
}

interface GitHubService {
    @GET("repos/topjohnwu/Magisk/contributors")
    @Headers("Accept: application/vnd.github+json", "X-GitHub-Api-Version: 2022-11-28")
    suspend fun getContributors(@Query("per_page") perPage: Int = 30): List<Map<String, Any?>>
}

data class HomeUiState(
    val magiskState: HomeViewModel.State = HomeViewModel.State.INVALID,
    val magiskInstalledVersion: String = AppContext.getString(CoreR.string.not_available),
    val appState: HomeViewModel.State = HomeViewModel.State.LOADING,
    val managerRemoteVersion: String = AppContext.getString(CoreR.string.not_available),
    val managerReleaseNotes: String = "",
    val managerInstalledVersion: String = "",
    val updateChannelName: String = AppContext.getString(CoreR.string.settings_update_stable),
    val packageName: String = "",
    val envActive: Boolean = Info.env.isActive,
    val contributors: List<Contributor> = emptyList(),
    val contributorsLoading: Boolean = true,
    val noticeVisible: Boolean = Config.safetyNotice
)

class HomeComposeViewModel(private val svc: NetworkService) : ViewModel() {
    private val _state = MutableStateFlow(HomeUiState())
    val state: StateFlow<HomeUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    private var refreshJob: Job? = null
    private val gitHubService: GitHubService by lazy {
        Retrofit.Builder().baseUrl("https://api.github.com/")
            .addConverterFactory(MoshiConverterFactory.create()).build()
            .create(GitHubService::class.java)
    }

    private fun cachedContributors(): List<Contributor>? {
        val cached = contributorsCache
        val cachedAt = contributorsCacheTimestamp
        return cached.takeIf {
            cached.isNotEmpty() && System.currentTimeMillis() - cachedAt < CONTRIBUTORS_CACHE_TTL_MS
        }
    }

    private fun cacheContributors(list: List<Contributor>) {
        contributorsCache = list
        contributorsCacheTimestamp = System.currentTimeMillis()
    }

    fun refresh() {
        refreshJob?.cancel()
        refreshJob = viewModelScope.launch {
            _state.update {
                if (it.contributors.isEmpty()) it.copy(contributorsLoading = true) else it
            }
            val cached = cachedContributors()
            if (cached != null) {
                _state.update { it.copy(contributors = cached, contributorsLoading = false) }
            } else {
                launch {
                    runCatching { gitHubService.getContributors(perPage = 30) }
                        .onSuccess { raw ->
                            val fetched = raw.mapNotNull { item ->
                                val login = item["login"] as? String ?: return@mapNotNull null
                                createContributor(
                                    login = login,
                                    avatarUrl = item["avatar_url"] as? String ?: "",
                                    htmlUrl = item["html_url"] as? String ?: ""
                                )
                            }

                            val priorityOrder =
                                listOf("topjohnwu", "vvb2060", "yujincheng08", "rikkaw", "canyie")
                            val fetchedMap = fetched.associateBy { it.login.lowercase(Locale.US) }
                            val ordered = priorityOrder.mapNotNull { handle -> fetchedMap[handle] }
                            val finalList = ordered.ifEmpty { fetched }
                            cacheContributors(finalList)

                            _state.update {
                                it.copy(
                                    contributors = finalList,
                                    contributorsLoading = false
                                )
                            }
                        }
                        .onFailure {
                            _state.update {
                                it.copy(
                                    contributors = emptyList(),
                                    contributorsLoading = false
                                )
                            }
                        }
                }
            }
            val remote = Info.fetchUpdate(svc)
            val appState = when {
                remote == null -> HomeViewModel.State.INVALID
                BuildConfig.APP_VERSION_CODE < remote.versionCode -> HomeViewModel.State.OUTDATED
                else -> HomeViewModel.State.UP_TO_DATE
            }
            _state.update {
                it.copy(
                    magiskState = if (Info.env.isActive) HomeViewModel.State.UP_TO_DATE else HomeViewModel.State.INVALID,
                    magiskInstalledVersion = if (Info.env.isActive) "${Info.env.versionString} (${Info.env.versionCode})" else AppContext.getString(
                        CoreR.string.not_available
                    ),
                    appState = appState,
                    managerInstalledVersion = BuildConfig.APP_VERSION_NAME,
                    managerRemoteVersion = remote?.version
                        ?: AppContext.getString(CoreR.string.not_available),
                    managerReleaseNotes = remote?.note.orEmpty(),
                    updateChannelName = AppContext.resources.getStringArray(CoreR.array.update_channel)
                        .getOrElse(Config.updateChannel) { AppContext.getString(CoreR.string.settings_update_stable) },
                    packageName = AppContext.packageName,
                    envActive = Info.env.isActive,
                    noticeVisible = Config.safetyNotice
                )
            }
        }
    }

    fun hideNotice() {
        Config.safetyNotice = false; _state.update { it.copy(noticeVisible = false) }
    }

    fun checkForMagiskUpdates() {
        refresh()
    }

    fun onManagerPressed(onShowInstallSheet: () -> Unit) {
        when (_state.value.appState) {
            HomeViewModel.State.LOADING -> {
                _messages.tryEmit(AppContext.getString(CoreR.string.loading))
            }
            HomeViewModel.State.INVALID -> {
                _messages.tryEmit(AppContext.getString(CoreR.string.no_connection))
            }
            else -> onShowInstallSheet()
        }
    }

    fun startManagerInstall(c: android.content.Context) {
        val activity = c as? UIActivity<*>
        if (activity != null) activity.withPermission(REQUEST_INSTALL_PACKAGES) {
            if (it) DownloadEngine.startWithActivity(
                activity,
                activity.extension,
                Subject.App()
            )
        }
        else DownloadEngine.start(c.applicationContext, Subject.App())
    }

    fun restoreImages() {
        viewModelScope.launch {
            _messages.tryEmit(AppContext.getString(CoreR.string.restore_img_msg))
            val success = MagiskInstaller.Restore().exec { }
            _messages.emit(AppContext.getString(if (success) CoreR.string.restore_done else CoreR.string.restore_fail))
        }
    }

    fun openLink(c: android.content.Context, l: String) {
        try {
            c.startActivity(
                Intent(
                    Intent.ACTION_VIEW,
                    Uri.parse(l)
                ).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            )
        } catch (_: Exception) {
            _messages.tryEmit(AppContext.getString(CoreR.string.open_link_failed_toast))
        }
    }

    companion object {
        private const val CONTRIBUTORS_CACHE_TTL_MS = 30L * 60_000L
        private var contributorsCache: List<Contributor> = emptyList()
        private var contributorsCacheTimestamp: Long = 0
    }

    object Factory : ViewModelProvider.Factory {
        override fun <T : ViewModel> create(modelClass: Class<T>): T {
            @Suppress("UNCHECKED_CAST") return HomeComposeViewModel(ServiceLocator.networkService) as T
        }
    }
}
