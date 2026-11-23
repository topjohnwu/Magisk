package com.topjohnwu.magisk.wear.home

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.BugReport
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.Extension
import androidx.compose.material.icons.filled.Security
import androidx.compose.material.icons.filled.Settings
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation3.runtime.NavKey
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.OutlinedButton
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import com.topjohnwu.magisk.core.R as CoreR

private class Shortcut(val label: Int, val icon: ImageVector, val key: NavKey)

private val shortcuts = listOf(
    Shortcut(CoreR.string.superuser, Icons.Filled.Security, WearRoute.Superuser),
    Shortcut(CoreR.string.modules, Icons.Filled.Extension, WearRoute.Modules),
    Shortcut(CoreR.string.logs, Icons.Filled.BugReport, WearRoute.Logs),
    Shortcut(CoreR.string.settings, Icons.Filled.Settings, WearRoute.Settings),
)

@Composable
fun HomeScreen(
    navigator: WearNavigator,
    viewModel: HomeViewModel = viewModel(factory = VMFactory),
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()

    // Reload whenever the screen is shown again, like the phone app does when its tab is selected
    LaunchedEffect(viewModel) {
        viewModel.startLoading()
    }
    CollectNavEvents(viewModel, navigator)
    EnvFixDialog(uiState.envFixCode, viewModel, navigator)

    // Kept across the loading screen, so the list doesn't scroll back when it ends
    val state = rememberTransformingLazyColumnState()

    if (uiState.showLoading) {
        // Fixing the environment runs as root and then reboots, so block the screen meanwhile
        LoadingScreen()
        return
    }

    // Recomposed whenever uiState changes, so this follows every reload
    val status = HomeStatus.from(Info.env, Info.isZygiskEnabled, Info.ramdisk)

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.magisk)) }
            }
            item {
                StatusCard(status, uiState.magiskState)
            }
            if (status.installed) {
                shortcuts.forEach { shortcut ->
                    item {
                        FilledTonalButton(
                            onClick = { navigator.navigate(shortcut.key) },
                            icon = { Icon(shortcut.icon, contentDescription = null) },
                            label = { Text(stringResource(shortcut.label)) },
                            modifier = Modifier.fillMaxWidth(),
                        )
                    }
                }
            }
            item {
                Button(
                    onClick = { navigator.navigate(WearRoute.Install) },
                    icon = { Icon(Icons.Filled.Download, contentDescription = null) },
                    label = { Text(stringResource(CoreR.string.install)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            if (status.installed) {
                item {
                    OutlinedButton(
                        onClick = { navigator.navigate(WearRoute.Restore) },
                        icon = { Icon(Icons.Filled.Delete, contentDescription = null) },
                        label = { Text(stringResource(CoreR.string.wear_home_uninstall)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
        }
    }
}

/** Asks to fix the environment when the check run by [HomeViewModel] reports a problem. */
@Composable
private fun EnvFixDialog(code: Int, viewModel: HomeViewModel, navigator: WearNavigator) {
    // Survives recreation once the event is consumed, like the phone app's dialog state
    var shownCode by rememberSaveable { mutableIntStateOf(0) }
    var visible by rememberSaveable { mutableStateOf(false) }

    LaunchedEffect(code) {
        if (code != 0) {
            shownCode = code
            visible = true
            viewModel.onEnvFixConsumed()
        }
    }

    // Same rule as the phone app: a mismatched version can only be fixed by reinstalling
    val needsFullFix = shownCode == 2 ||
        Info.env.versionCode != BuildConfig.APP_VERSION_CODE ||
        Info.env.versionString != BuildConfig.APP_VERSION_NAME

    AlertDialog(
        visible = visible,
        onDismissRequest = { visible = false },
        title = { Text(stringResource(CoreR.string.env_fix_title)) },
        text = {
            Text(
                stringResource(
                    if (needsFullFix) CoreR.string.env_full_fix_msg else CoreR.string.env_fix_msg
                )
            )
        },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                visible = false
                if (needsFullFix) navigator.navigate(WearRoute.Install) else viewModel.fixEnv()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { visible = false })
        },
    )
}

@Composable
private fun StatusCard(status: HomeStatus, magiskState: HomeViewModel.State) {
    val colors = MaterialTheme.colorScheme
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .background(colors.surfaceContainer, MaterialTheme.shapes.large)
            .padding(horizontal = 16.dp, vertical = 12.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        if (status.installed) {
            Row(
                horizontalArrangement = Arrangement.spacedBy(8.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(
                    text = status.version ?: stringResource(CoreR.string.not_available),
                    style = MaterialTheme.typography.titleLarge,
                )
                Text(
                    text = stringResource(
                        if (magiskState == HomeViewModel.State.OUTDATED) CoreR.string.update_available
                        else CoreR.string.wear_home_running
                    ),
                    style = MaterialTheme.typography.labelSmall,
                    color = colors.primary,
                )
            }
            status.versionCode?.let {
                Text(
                    text = stringResource(CoreR.string.wear_home_installed_code, it),
                    style = MaterialTheme.typography.bodySmall,
                    color = colors.onSurfaceVariant,
                )
            }
        } else {
            Text(
                text = stringResource(CoreR.string.not_installed),
                style = MaterialTheme.typography.titleMedium,
                color = colors.error,
            )
        }
        Text(
            text = stringResource(
                CoreR.string.wear_home_status_item,
                stringResource(CoreR.string.zygisk),
                stringResource(if (status.zygisk) CoreR.string.enabled else CoreR.string.disabled),
            ),
            style = MaterialTheme.typography.bodySmall,
            color = colors.onSurfaceVariant,
        )
        Text(
            text = stringResource(
                CoreR.string.wear_home_status_item,
                stringResource(CoreR.string.ramdisk),
                stringResource(if (status.ramdisk) CoreR.string.yes else CoreR.string.no),
            ),
            style = MaterialTheme.typography.bodySmall,
            color = colors.onSurfaceVariant,
        )
    }
}
