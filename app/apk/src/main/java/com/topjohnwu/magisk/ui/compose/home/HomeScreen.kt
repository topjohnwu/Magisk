package com.topjohnwu.magisk.ui.compose.home

import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.content.Intent
import android.net.Uri
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.compose.animation.*
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
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
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
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.flow.MutableStateFlow
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
    onOpenInstall: () -> Unit = {},
    onOpenUninstall: () -> Unit = {},
    viewModel: HomeComposeViewModel = viewModel(factory = HomeComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val context = LocalContext.current
    val snackbarHostState = remember { SnackbarHostState() }
    var showUninstallDialog by remember { mutableStateOf(false) }
    var showRebootDialog by remember { mutableStateOf(false) }
    val lifecycleOwner = LocalLifecycleOwner.current

    LaunchedEffect(Unit) { viewModel.refresh() }
    LaunchedEffect(state.message) {
        state.message?.let {
            snackbarHostState.showSnackbar(it)
            viewModel.consumeMessage()
        }
    }

    DisposableEffect(lifecycleOwner) {
        val observer = LifecycleEventObserver { _, event ->
            if (event == Lifecycle.Event.ON_RESUME) viewModel.refresh()
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(bottom = 140.dp)
        ) {
            ExpressiveHeader(state, onRebootClick = { showRebootDialog = true })

            Column(
                modifier = Modifier.padding(horizontal = 24.dp),
                verticalArrangement = Arrangement.spacedBy(24.dp)
            ) {
                if (state.noticeVisible) {
                    NoticeCard(onHide = viewModel::hideNotice)
                }

                SectionHeader(stringResource(id = CoreR.string.home_section_magisk_core), Icons.Rounded.VerifiedUser)
                MagiskOrganicCard(state, onAction = onOpenInstall)
                
                SectionHeader(stringResource(id = CoreR.string.home_section_application), Icons.Rounded.AppShortcut)
                AppOrganicCardXL(state, onAction = { viewModel.onManagerPressed(context) })

                if (state.envActive) {
                    UninstallAction(onClick = { showUninstallDialog = true })
                }

                SectionHeader(stringResource(id = CoreR.string.home_section_contributors), Icons.Rounded.Groups)
                ContributorsExpressiveList(state.contributors, state.contributorsLoading, onOpen = { viewModel.openLink(context, it) })

                SectionHeader(stringResource(id = CoreR.string.home_support_title), Icons.Rounded.Favorite)
                SupportOrganicSection(
                    onPatreon = { viewModel.openLink(context, Const.Url.PATREON_URL) },
                    onPaypal = { viewModel.openLink(context, Const.Url.PAYPAL_URL) }
                )
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 120.dp)
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
}

@Composable
private fun ExpressiveHeader(state: HomeUiState, onRebootClick: () -> Unit) {
    val isInstalled = state.envActive
    val primaryColor = if (isInstalled) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.error
    val containerColor = if (isInstalled) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.errorContainer
    val title = stringResource(id = if (isInstalled) CoreR.string.home_status_ready else CoreR.string.home_status_inactive)
    val subtitle = stringResource(id = if (isInstalled) CoreR.string.home_status_ready_subtitle else CoreR.string.home_status_inactive_subtitle)

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = 16.dp, end = 16.dp, bottom = 16.dp)
    ) {
        Surface(
            modifier = Modifier
                .fillMaxWidth()
                .height(240.dp),
            shape = RoundedCornerShape(topStart = 48.dp, topEnd = 120.dp, bottomStart = 120.dp, bottomEnd = 48.dp),
            color = containerColor,
            tonalElevation = 6.dp
        ) {
            Box(modifier = Modifier.fillMaxSize()) {
                Icon(
                    painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier
                        .size(220.dp)
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
                        shape = RoundedCornerShape(16.dp),
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
                                modifier = Modifier.size(18.dp),
                                tint = primaryColor
                            )
                            Text(
                                text = if (isInstalled) stringResource(id = CoreR.string.home_status_ready) else stringResource(id = CoreR.string.home_status_inactive),
                                style = MaterialTheme.typography.labelMedium,
                                fontWeight = FontWeight.ExtraBold,
                                color = primaryColor
                            )
                        }
                    }
                    
                    Spacer(Modifier.height(16.dp))
                    
                    Text(
                        text = title,
                        style = MaterialTheme.typography.headlineLarge,
                        fontWeight = FontWeight.ExtraBold,
                        color = primaryColor,
                        lineHeight = 40.sp
                    )
                    Text(
                        text = subtitle,
                        style = MaterialTheme.typography.titleMedium,
                        color = primaryColor.copy(alpha = 0.8f),
                        fontWeight = FontWeight.SemiBold
                    )
                }

                Surface(
                    onClick = onRebootClick,
                    modifier = Modifier
                        .align(Alignment.TopEnd)
                        .padding(24.dp)
                        .size(56.dp),
                    shape = RoundedCornerShape(18.dp),
                    color = primaryColor.copy(alpha = 0.1f)
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        Icon(
                            Icons.Rounded.PowerSettingsNew, 
                            contentDescription = stringResource(id = CoreR.string.reboot),
                            tint = primaryColor,
                            modifier = Modifier.size(28.dp)
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun MagiskOrganicCard(state: HomeUiState, onAction: () -> Unit) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(topStart = 48.dp, bottomEnd = 48.dp, topEnd = 12.dp, bottomStart = 12.dp),
        colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHighest)
    ) {
        Box {
            Icon(
                painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                contentDescription = null,
                modifier = Modifier.size(140.dp).align(Alignment.TopEnd).offset(x = 30.dp, y = (-20).dp).alpha(0.05f),
                tint = MaterialTheme.colorScheme.primary
            )
            
            Column(modifier = Modifier.padding(28.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Surface(color = MaterialTheme.colorScheme.primary, shape = RoundedCornerShape(16.dp), modifier = Modifier.size(52.dp)) {
                        Icon(painter = painterResource(id = CoreR.drawable.ic_magisk), null, modifier = Modifier.padding(12.dp), tint = MaterialTheme.colorScheme.onPrimary)
                    }
                    Spacer(Modifier.width(16.dp))
                    Column(Modifier.weight(1f)) {
                        Text(stringResource(id = CoreR.string.magisk), style = MaterialTheme.typography.titleLarge, fontWeight = FontWeight.ExtraBold)
                        StatusBadge(state.magiskState)
                    }
                }
                Spacer(Modifier.height(24.dp))
                BentoInfoGrid(
                    listOf(
                        stringResource(id = CoreR.string.home_installed_version) to state.magiskInstalledVersion,
                        stringResource(id = CoreR.string.zygisk) to stringResource(id = if (Info.isZygiskEnabled) CoreR.string.yes else CoreR.string.no),
                        stringResource(id = CoreR.string.home_ramdisk) to stringResource(id = if (Info.ramdisk) CoreR.string.yes else CoreR.string.no)
                    )
                )
                Spacer(Modifier.height(24.dp))
                Button(onClick = onAction, modifier = Modifier.fillMaxWidth().height(60.dp), shape = RoundedCornerShape(20.dp)) {
                    Icon(Icons.Rounded.FlashOn, null)
                    Spacer(Modifier.width(12.dp))
                    Text(
                        stringResource(id = if (state.magiskState == HomeViewModel.State.OUTDATED) CoreR.string.home_update_magisk else CoreR.string.home_maintain),
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }
    }
}

@Composable
private fun AppOrganicCardXL(state: HomeUiState, onAction: () -> Unit) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(topStart = 12.dp, bottomEnd = 12.dp, topEnd = 64.dp, bottomStart = 64.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
    ) {
        Column(modifier = Modifier.padding(32.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Surface(color = MaterialTheme.colorScheme.secondary, shape = RoundedCornerShape(20.dp), modifier = Modifier.size(72.dp)) {
                    Icon(Icons.Rounded.AppShortcut, null, modifier = Modifier.padding(18.dp), tint = MaterialTheme.colorScheme.onSecondary)
                }
                Spacer(Modifier.width(20.dp))
                Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                        Text(stringResource(id = CoreR.string.home_app_title), style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Black)
                        if (state.appState != HomeViewModel.State.INVALID && state.appState != HomeViewModel.State.LOADING) {
                            StatusBadge(state.appState)
                        }
                    }
                    Text(state.managerInstalledVersion, style = MaterialTheme.typography.titleSmall, color = MaterialTheme.colorScheme.primary, fontWeight = FontWeight.Bold)
                }
            }
            Spacer(Modifier.height(32.dp))
            BentoInfoGrid(
                listOf(
                    stringResource(id = CoreR.string.home_latest_version) to state.managerRemoteVersion,
                    stringResource(id = CoreR.string.home_channel) to state.updateChannelName,
                    stringResource(id = CoreR.string.home_package) to state.packageName
                )
            )
            if (state.appState != HomeViewModel.State.INVALID && state.appState != HomeViewModel.State.LOADING) {
                Spacer(Modifier.height(32.dp))
                Button(onClick = onAction, modifier = Modifier.fillMaxWidth().height(64.dp), shape = RoundedCornerShape(20.dp), colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)) {
                    Icon(Icons.Rounded.Update, null)
                    Spacer(Modifier.width(12.dp))
                    Text(
                        if (state.appState == HomeViewModel.State.OUTDATED) {
                            stringResource(id = CoreR.string.home_update_app_now)
                        } else {
                            stringResource(id = CoreR.string.home_reinstall_app)
                        },
                        fontWeight = FontWeight.Black,
                        fontSize = 16.sp
                    )
                }
            }
        }
    }
}

@Composable
private fun BentoInfoGrid(items: List<Pair<String, String>>) {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
        items.forEach { (label, value) ->
            Column(modifier = Modifier.weight(1f).clip(RoundedCornerShape(24.dp)).background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f)).padding(14.dp)) {
                Text(label, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.primary, fontWeight = FontWeight.Black)
                Text(value, style = MaterialTheme.typography.bodySmall, fontWeight = FontWeight.Bold, maxLines = 1, overflow = TextOverflow.Ellipsis)
            }
        }
    }
}

@Composable
private fun ContributorsExpressiveList(contributors: List<Contributor>, loading: Boolean, onOpen: (String) -> Unit) {
    if (loading && contributors.isEmpty()) {
        LinearProgressIndicator(modifier = Modifier.fillMaxWidth().padding(vertical = 20.dp).clip(CircleShape), strokeCap = StrokeCap.Round)
        return
    }

    if (contributors.isEmpty()) return

    val shapes = listOf(
        RoundedCornerShape(topStart = 64.dp, topEnd = 16.dp, bottomStart = 16.dp, bottomEnd = 64.dp),
        RoundedCornerShape(topStart = 16.dp, topEnd = 64.dp, bottomStart = 64.dp, bottomEnd = 16.dp),
        RoundedCornerShape(topStart = 48.dp, topEnd = 48.dp, bottomStart = 12.dp, bottomEnd = 48.dp),
        RoundedCornerShape(topStart = 12.dp, topEnd = 56.dp, bottomStart = 56.dp, bottomEnd = 56.dp)
    )

    val maintainerHandles = MAINTAINER_LINKS.keys
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .horizontalScroll(rememberScrollState()),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        contributors.forEachIndexed { index, user ->
            val shape = shapes[index % shapes.size]
            val isMaintainer = maintainerHandles.contains(user.login.lowercase(Locale.US))
            ElevatedCard(
                modifier = Modifier
                    .width(160.dp)
                    .height(200.dp)
                    .clip(shape)
                    .clickable { onOpen(user.htmlUrl) },
                shape = shape,
                colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(
                            Brush.linearGradient(
                                listOf(
                                    MaterialTheme.colorScheme.surfaceContainer,
                                    MaterialTheme.colorScheme.primary.copy(alpha = 0.06f)
                                )
                            )
                        )
                        .padding(16.dp)
                ) {
                    Column(
                        modifier = Modifier.fillMaxSize(),
                        horizontalAlignment = Alignment.CenterHorizontally,
                        verticalArrangement = Arrangement.spacedBy(10.dp)
                    ) {
                        AsyncImage(
                            model = user.avatarUrl,
                            contentDescription = null,
                            modifier = Modifier.size(64.dp).clip(CircleShape),
                            contentScale = ContentScale.Crop
                        )

                        Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(4.dp)) {
                            Text(user.login, style = MaterialTheme.typography.titleSmall, fontWeight = FontWeight.Black, textAlign = TextAlign.Center, maxLines = 1, overflow = TextOverflow.Ellipsis)
                            if (isMaintainer) {
                                Surface(
                                    color = MaterialTheme.colorScheme.primary.copy(alpha = 0.12f),
                                    shape = RoundedCornerShape(12.dp)
                                ) {
                                    Text(
                                        text = stringResource(id = CoreR.string.home_section_contributors),
                                        modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp),
                                        style = MaterialTheme.typography.labelSmall,
                                        color = MaterialTheme.colorScheme.primary,
                                        fontWeight = FontWeight.Bold
                                    )
                                }
                            }
                        }

                        Spacer(Modifier.weight(1f))

                        Row(
                            horizontalArrangement = Arrangement.spacedBy(8.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            user.links.take(3).forEach { link ->
                                Surface(
                                    modifier = Modifier
                                        .size(32.dp)
                                        .clip(CircleShape)
                                        .clickable { onOpen(link.url) },
                                    color = MaterialTheme.colorScheme.primary.copy(alpha = 0.12f)
                                ) {
                                    Icon(
                                        painter = painterResource(id = link.iconRes),
                                        contentDescription = stringResource(id = link.labelRes),
                                        modifier = Modifier.padding(8.dp),
                                        tint = MaterialTheme.colorScheme.primary
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
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(24.dp))
            .background(MaterialTheme.colorScheme.surfaceContainer)
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(id = CoreR.string.home_support_content),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
            Button(onClick = onPatreon, modifier = Modifier.weight(1f).height(64.dp), shape = RoundedCornerShape(topStart = 32.dp, bottomEnd = 32.dp, topEnd = 8.dp, bottomStart = 8.dp), colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFFF424D), contentColor = Color.White)) {
                Icon(Icons.Rounded.CloudQueue, null)
                Spacer(Modifier.width(8.dp))
                Text(stringResource(id = CoreR.string.patreon), fontWeight = FontWeight.Black)
            }
            Button(onClick = onPaypal, modifier = Modifier.weight(1f).height(64.dp), shape = RoundedCornerShape(topEnd = 32.dp, bottomStart = 32.dp, topStart = 8.dp, bottomEnd = 8.dp), colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF003087), contentColor = Color.White)) {
                Icon(Icons.Rounded.Payments, null)
                Spacer(Modifier.width(8.dp))
                Text(stringResource(id = CoreR.string.paypal), fontWeight = FontWeight.Black)
            }
        }
    }
}

@Composable
private fun SectionHeader(title: String, icon: ImageVector) {
    Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(start = 4.dp)) {
        Icon(icon, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.primary)
        Spacer(Modifier.width(12.dp))
        Text(text = title.uppercase(), style = MaterialTheme.typography.labelLarge, fontWeight = FontWeight.Black, letterSpacing = 1.5.sp, color = MaterialTheme.colorScheme.outline)
    }
}

@Composable
private fun StatusBadge(state: HomeViewModel.State) {
    val (text, color) = when (state) {
        HomeViewModel.State.UP_TO_DATE -> stringResource(id = CoreR.string.home_state_up_to_date) to Color(0xFF4CAF50)
        HomeViewModel.State.OUTDATED -> stringResource(id = CoreR.string.home_state_update_ready) to Color(0xFFFF9800)
        else -> stringResource(id = CoreR.string.home_state_inactive) to MaterialTheme.colorScheme.error
    }
    Surface(color = color.copy(alpha = 0.15f), shape = RoundedCornerShape(12.dp)) {
        Text(text = text, modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp), style = MaterialTheme.typography.labelSmall, color = color, fontWeight = FontWeight.Black)
    }
}

@Composable
private fun UninstallAction(onClick: () -> Unit) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(24.dp),
        color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.2f),
        border = androidx.compose.foundation.BorderStroke(1.dp, MaterialTheme.colorScheme.error.copy(alpha = 0.1f)),
        onClick = onClick
    ) {
        Row(modifier = Modifier.padding(20.dp), verticalAlignment = Alignment.CenterVertically) {
            Icon(Icons.Rounded.DeleteForever, null, tint = MaterialTheme.colorScheme.error)
            Spacer(Modifier.width(16.dp))
            Text(stringResource(id = CoreR.string.home_uninstall_environment), style = MaterialTheme.typography.titleSmall, fontWeight = FontWeight.Black, color = MaterialTheme.colorScheme.error)
        }
    }
}

@Composable
private fun NoticeCard(onHide: () -> Unit) {
    Surface(modifier = Modifier.fillMaxWidth(), shape = RoundedCornerShape(32.dp), color = MaterialTheme.colorScheme.tertiaryContainer.copy(alpha = 0.6f)) {
        Row(modifier = Modifier.padding(24.dp), verticalAlignment = Alignment.CenterVertically) {
            Icon(Icons.Rounded.Info, null, tint = MaterialTheme.colorScheme.onTertiaryContainer)
            Spacer(Modifier.width(16.dp))
            Text(stringResource(id = CoreR.string.home_notice_content), modifier = Modifier.weight(1f), style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onTertiaryContainer, lineHeight = 20.sp)
            IconButton(onClick = onHide) { Icon(Icons.Rounded.Close, null, modifier = Modifier.size(20.dp)) }
        }
    }
}

@Composable
private fun RebootExpressiveDialog(onDismiss: () -> Unit, onReboot: (String) -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Row(verticalAlignment = Alignment.CenterVertically) { Icon(Icons.Rounded.RestartAlt, null, tint = MaterialTheme.colorScheme.primary); Spacer(Modifier.width(12.dp)); Text(stringResource(id = CoreR.string.reboot), fontWeight = FontWeight.Black) } },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                ExpressiveOptionCard(Icons.Rounded.PowerSettingsNew, stringResource(id = CoreR.string.reboot), subtitle = null, accentColor = Color(0xFF4CAF50)) { onReboot("") }
                ExpressiveOptionCard(Icons.Rounded.History, stringResource(id = CoreR.string.reboot_recovery), subtitle = null, accentColor = Color(0xFFFF9800)) { onReboot("recovery") }
                ExpressiveOptionCard(Icons.Rounded.Terminal, stringResource(id = CoreR.string.reboot_bootloader), subtitle = null, accentColor = Color(0xFF2196F3)) { onReboot("bootloader") }
                ExpressiveOptionCard(Icons.Rounded.Download, stringResource(id = CoreR.string.reboot_download), subtitle = null, accentColor = Color(0xFF9C27B0)) { onReboot("download") }
                ExpressiveOptionCard(Icons.Rounded.Bolt, stringResource(id = CoreR.string.reboot_userspace), subtitle = null, accentColor = Color(0xFFFFEB3B)) { onReboot("userspace") }
            }
        },
        confirmButton = { TextButton(onClick = onDismiss) { Text(stringResource(id = CoreR.string.close), fontWeight = FontWeight.Bold) } },
        shape = RoundedCornerShape(32.dp),
        containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
    )
}

@Composable
private fun ExpressiveOptionCard(icon: ImageVector, title: String, subtitle: String?, accentColor: Color, onClick: () -> Unit) {
    Card(modifier = Modifier.fillMaxWidth().clickable { onClick() }, shape = RoundedCornerShape(topEnd = 24.dp, bottomStart = 24.dp, topStart = 8.dp, bottomEnd = 8.dp), colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Surface(color = accentColor.copy(alpha = 0.15f), shape = CircleShape, modifier = Modifier.size(44.dp)) {
                Icon(icon, null, modifier = Modifier.padding(10.dp), tint = accentColor)
            }
            Spacer(Modifier.width(16.dp))
            Column {
                Text(title, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.ExtraBold)
                if (subtitle != null) {
                    Text(subtitle, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
                }
            }
        }
    }
}

@Composable
private fun UninstallExpressiveDialog(onDismiss: () -> Unit, onRestoreImages: () -> Unit, onCompleteUninstall: () -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Row(verticalAlignment = Alignment.CenterVertically) { Icon(Icons.Rounded.DeleteSweep, null, tint = MaterialTheme.colorScheme.error); Spacer(Modifier.width(12.dp)); Text(stringResource(id = CoreR.string.uninstall_magisk_title), fontWeight = FontWeight.Black) } },
        text = { Text(stringResource(id = CoreR.string.uninstall_magisk_msg)) },
        confirmButton = {
            Column(verticalArrangement = Arrangement.spacedBy(12.dp), modifier = Modifier.fillMaxWidth().padding(top = 16.dp)) {
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
data class ContributorLink(@StringRes val labelRes: Int, @DrawableRes val iconRes: Int, val url: String)
data class Contributor(val login: String, val avatarUrl: String, val htmlUrl: String, val links: List<ContributorLink> = emptyList())

private val MAINTAINER_LINKS: Map<String, List<ContributorLink>> = mapOf(
    "topjohnwu" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/topjohnwu"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/topjohnwu/Magisk")
    ),
    "vvb2060" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/vvb2060"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/vvb2060")
    ),
    "yujincheng08" to listOf(
        ContributorLink(CoreR.string.twitter, CoreR.drawable.ic_twitter, "https://x.com/yujincheng08"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_github, "https://github.com/yujincheng08"),
        ContributorLink(CoreR.string.github, CoreR.drawable.ic_favorite, "https://github.com/sponsors/yujincheng08")
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

data class HomeUiState(val loading: Boolean = true, val magiskState: HomeViewModel.State = HomeViewModel.State.INVALID, val magiskInstalledVersion: String = AppContext.getString(CoreR.string.not_available), val appState: HomeViewModel.State = HomeViewModel.State.LOADING, val managerRemoteVersion: String = AppContext.getString(CoreR.string.not_available), val managerInstalledVersion: String = "", val updateChannelName: String = AppContext.getString(CoreR.string.settings_update_stable), val packageName: String = "", val envActive: Boolean = Info.env.isActive, val contributors: List<Contributor> = emptyList(), val contributorsLoading: Boolean = true, val noticeVisible: Boolean = Config.safetyNotice, val message: String? = null)

class HomeComposeViewModel(private val svc: NetworkService) : ViewModel() {
    private val _state = MutableStateFlow(HomeUiState())
    val state: StateFlow<HomeUiState> = _state
    private val gitHubService: GitHubService by lazy { Retrofit.Builder().baseUrl("https://api.github.com/").addConverterFactory(MoshiConverterFactory.create()).build().create(GitHubService::class.java) }

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
        viewModelScope.launch {
            _state.update { it.copy(loading = true, contributorsLoading = true) }
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

                            val priorityOrder = listOf("topjohnwu", "vvb2060", "yujincheng08", "rikkaw", "canyie")
                            val fetchedMap = fetched.associateBy { it.login.lowercase(Locale.US) }
                            val ordered = priorityOrder.mapNotNull { handle -> fetchedMap[handle] }
                            val finalList = ordered.ifEmpty { fetched }
                            cacheContributors(finalList)

                            _state.update { it.copy(contributors = finalList, contributorsLoading = false) }
                        }
                        .onFailure { _state.update { it.copy(contributors = emptyList(), contributorsLoading = false) } }
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
                    loading = false,
                    magiskState = if (Info.env.isActive) HomeViewModel.State.UP_TO_DATE else HomeViewModel.State.INVALID,
                    magiskInstalledVersion = if (Info.env.isActive) "${Info.env.versionString} (${Info.env.versionCode})" else AppContext.getString(CoreR.string.not_available),
                    appState = appState,
                    managerInstalledVersion = BuildConfig.APP_VERSION_NAME,
                    managerRemoteVersion = remote?.version ?: AppContext.getString(CoreR.string.not_available),
                    updateChannelName = AppContext.resources.getStringArray(CoreR.array.update_channel).getOrElse(Config.updateChannel) { AppContext.getString(CoreR.string.settings_update_stable) },
                    packageName = AppContext.packageName,
                    envActive = Info.env.isActive,
                    noticeVisible = Config.safetyNotice
                )
            }
        }
    }
    fun hideNotice() { Config.safetyNotice = false; _state.update { it.copy(noticeVisible = false) } }
    fun checkForMagiskUpdates() { refresh() }
    fun onManagerPressed(c: android.content.Context) {
        val activity = c as? UIActivity<*>
        if (activity != null) activity.withPermission(REQUEST_INSTALL_PACKAGES) { if (it) DownloadEngine.startWithActivity(activity, Subject.App()) }
        else DownloadEngine.start(c.applicationContext, Subject.App())
    }
    fun restoreImages() { viewModelScope.launch { val success = MagiskInstaller.Restore().exec { }; _state.update { it.copy(message = AppContext.getString(if (success) CoreR.string.restore_done else CoreR.string.restore_fail)) } } }
    fun consumeMessage() { _state.update { it.copy(message = null) } }
    fun openLink(c: android.content.Context, l: String) { try { c.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(l)).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)) } catch (_: Exception) {} }

    companion object {
        private const val CONTRIBUTORS_CACHE_TTL_MS = 30L * 60_000L
        private var contributorsCache: List<Contributor> = emptyList()
        private var contributorsCacheTimestamp: Long = 0
    }

    object Factory : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return HomeComposeViewModel(ServiceLocator.networkService) as T } }
}
