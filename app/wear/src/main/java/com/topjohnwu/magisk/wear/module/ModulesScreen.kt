package com.topjohnwu.magisk.wear.module

import android.content.Context
import android.net.Uri
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.RestartAlt
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.IconButton
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SplitSwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.ui.flash.queryDisplayName
import com.topjohnwu.magisk.ui.flash.sanitizeDisplayName
import com.topjohnwu.magisk.ui.module.ModuleItem
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.files.FileKind
import com.topjohnwu.magisk.wear.files.rememberFilePicker
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

// Bits of the state of a module that only take effect after a reboot
private const val STATE_ENABLED = 1
private const val STATE_REMOVED = 2

private val ModuleItem.pendingState: Int
    get() = (if (isEnabled) STATE_ENABLED else 0) or (if (isRemoved) STATE_REMOVED else 0)

/**
 * The installed modules. [viewModel] is shared with [ModuleDetailScreen], so changes made there
 * show up here right away.
 *
 * [loadedStates] keeps the state of each module (by directory) when it was first loaded. Changes
 * are written to disk right away, so a reload can't tell them apart; this is what stays active
 * until the next reboot. It is filled in place, and kept as long as [viewModel], so leaving this
 * screen doesn't hide pending changes.
 */
@Composable
fun ModulesScreen(
    viewModel: ModuleViewModel,
    navigator: WearNavigator,
    loadedStates: MutableMap<String, Int>,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val context = LocalContext.current
    val resources = LocalResources.current
    val scope = rememberCoroutineScope()

    // Reload whenever the screen is shown again, like the phone app does when its tab is selected
    LaunchedEffect(viewModel) {
        viewModel.startLoading()
    }
    CollectNavEvents(viewModel, navigator)

    // Record every module seen for the first time before comparing, so none is skipped
    uiState.modules.forEach { loadedStates.getOrPut(it.module.base.name) { it.pendingState } }
    val needsReboot = uiState.modules.any { loadedStates[it.module.base.name] != it.pendingState }
    var showReboot by rememberSaveable { mutableStateOf(false) }

    var pendingInstall by remember { mutableStateOf<Uri?>(null) }
    // Not cleared, so the text stays while the dialog animates out after it is answered
    var installMessage by remember { mutableStateOf("") }
    val pickZip = rememberFilePicker(FileKind.ZIP) { uri ->
        scope.launch {
            val name = displayName(context, uri)
            installMessage = resources.getString(CoreR.string.confirm_install, name)
            pendingInstall = uri
        }
    }

    // Kept across the loading screen, so the list doesn't scroll back when it ends
    val state = rememberTransformingLazyColumnState()

    if (uiState.loading) {
        LoadingScreen()
        return
    }

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item(key = "header") {
                ListHeader {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(stringResource(CoreR.string.modules))
                        IconButton(onClick = pickZip) {
                            Icon(
                                Icons.Filled.Add,
                                contentDescription = stringResource(
                                    CoreR.string.module_action_install_external
                                ),
                            )
                        }
                    }
                }
            }
            if (needsReboot) {
                item(key = "reboot") {
                    FilledTonalButton(
                        onClick = { showReboot = true },
                        icon = { Icon(Icons.Filled.RestartAlt, contentDescription = null) },
                        label = { Text(stringResource(CoreR.string.wear_module_reboot)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
            if (uiState.modules.isEmpty()) {
                item(key = "empty") {
                    Text(
                        text = stringResource(CoreR.string.module_empty),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
                    )
                }
            } else {
                items(
                    items = uiState.modules,
                    // Module ids come from module.prop and can be duplicated, directory names can't
                    key = { it.module.base.name },
                ) { item ->
                    ModuleRow(
                        item = item,
                        onToggle = { viewModel.toggleEnabled(item) },
                        onDetail = {
                            navigator.navigate(WearRoute.ModuleDetail(item.module.base.name))
                        },
                    )
                }
            }
        }
    }

    AlertDialog(
        visible = showReboot,
        onDismissRequest = { showReboot = false },
        title = { Text(stringResource(CoreR.string.reboot_apply_change)) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                showReboot = false
                reboot()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { showReboot = false })
        },
    )

    AlertDialog(
        visible = pendingInstall != null,
        onDismissRequest = { pendingInstall = null },
        title = { Text(stringResource(CoreR.string.confirm_install_title)) },
        text = { Text(installMessage) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                pendingInstall?.let { viewModel.confirmLocalInstall(it) }
                pendingInstall = null
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { pendingInstall = null })
        },
    )
}

@Composable
private fun ModuleRow(
    item: ModuleItem,
    onToggle: () -> Unit,
    onDetail: () -> Unit,
) {
    val module = item.module
    val status = when {
        item.isRemoved -> stringResource(CoreR.string.wear_module_removed)
        !item.isEnabled -> stringResource(CoreR.string.wear_module_disabled)
        item.showNotice -> item.noticeText.getText(LocalResources.current)
        else -> module.description
    }
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None

    SplitSwitchButton(
        checked = item.isEnabled,
        onCheckedChange = { onToggle() },
        toggleContentDescription = module.name,
        onContainerClick = onDetail,
        modifier = Modifier.fillMaxWidth(),
        label = {
            Column(
                modifier = Modifier.weight(1f),
                verticalArrangement = Arrangement.Center,
            ) {
                Text(
                    text = module.name,
                    textDecoration = strikeThrough,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                Text(
                    text = if (status.isBlank()) {
                        module.version
                    } else {
                        stringResource(CoreR.string.wear_module_subtitle, module.version, status)
                    },
                    style = MaterialTheme.typography.labelSmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
            }
        },
    )
}

/**
 * The name to show for a picked file, for the install confirmation. The name is chosen by the
 * app that provides the file, so it comes with where the file comes from, like for a request
 * sent by another app. Files copied by the root file list have no authority.
 */
private suspend fun displayName(context: Context, uri: Uri): String {
    val name = queryDisplayName(context, uri)?.sanitizeDisplayName()?.ifEmpty { null }
        ?: uri.lastPathSegment?.substringAfterLast('/')?.sanitizeDisplayName()?.ifEmpty { null }
        ?: "module.zip"
    val authority = uri.authority?.sanitizeDisplayName()?.ifEmpty { null }
    return authority?.let { "$name ($it)" } ?: name
}
