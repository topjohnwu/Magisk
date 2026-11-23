package com.topjohnwu.magisk.wear.settings

import android.Manifest.permission.POST_NOTIFICATIONS
import android.app.RemoteInput
import android.content.ActivityNotFoundException
import android.content.pm.PackageManager
import android.os.Build
import android.webkit.URLUtil
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Edit
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringArrayResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import androidx.lifecycle.compose.LifecycleResumeEffect
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.TransformingLazyColumnScope
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.ButtonDefaults
import androidx.wear.compose.material3.CompactButton
import androidx.wear.compose.material3.Dialog
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.RadioButton
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SplitSwitchButton
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import androidx.wear.input.RemoteInputIntentHelper
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.settings.SettingsDialog
import com.topjohnwu.magisk.ui.settings.SettingsUiState
import com.topjohnwu.magisk.ui.settings.SettingsViewModel
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.core.R as CoreR

private const val URL_INPUT_KEY = "url"

/**
 * The settings of the phone app that apply to a watch, shown under the same conditions.
 *
 * Left out on Wear:
 * - Language: the app follows the system language of the watch.
 * - Color mode: the Wear theme is always dark.
 * - Download path and randomized output name: patched images stay in the Download folder,
 *   which is where the root file list of the Wear file picker looks.
 * - Superuser authentication: the biometric prompt is not available in the Wear app.
 * - Multiuser mode: Wear OS has no secondary users.
 */
@Composable
fun SettingsScreen(viewModel: SettingsViewModel, navigator: WearNavigator) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val enableCheckUpdate = rememberNotificationPermissionLauncher {
        viewModel.setCheckUpdate(true)
    }

    LifecycleResumeEffect(viewModel) {
        viewModel.refreshEnvironment()
        onPauseOrDispose {}
    }
    CollectNavEvents(viewModel, navigator)

    val state = rememberTransformingLazyColumnState()
    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.settings)) }
            }
            if (uiState.isMagiskInstalled) {
                magiskItems(uiState, viewModel)
            }
            if (uiState.isSuperUserAvailable) {
                superuserItems(uiState, viewModel)
            }
            item {
                ListHeader { Text(stringResource(CoreR.string.wear_settings_updates)) }
            }
            item {
                SwitchButton(
                    checked = uiState.checkUpdateEnabled,
                    // Update notifications need the notification permission
                    onCheckedChange = { enabled ->
                        if (enabled) enableCheckUpdate() else viewModel.setCheckUpdate(false)
                    },
                    label = { Text(stringResource(CoreR.string.settings_check_update_title)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            item {
                ChannelPicker(
                    current = uiState.selectedUpdateChannelIndex,
                    onSelect = viewModel::setUpdateChannel,
                )
            }
            if (uiState.isCustomChannel) {
                item {
                    val url = uiState.customChannelUrl
                    FilledTonalButton(
                        onClick = viewModel::openCustomChannelUrlDialog,
                        label = { Text(stringResource(CoreR.string.settings_update_custom)) },
                        secondaryLabel = if (url.isNotBlank()) {
                            { Text(url, maxLines = 2) }
                        } else null,
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
            item {
                SwitchButton(
                    checked = uiState.dohEnabled,
                    onCheckedChange = viewModel::setDoh,
                    label = { Text(stringResource(CoreR.string.settings_doh_title)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }

    CustomChannelUrlDialog(
        dialog = uiState.activeDialog as? SettingsDialog.CustomChannelUrl,
        onSave = viewModel::saveCustomChannelUrl,
        onDismiss = viewModel::dismissDialog,
    )
}

private fun TransformingLazyColumnScope.magiskItems(
    uiState: SettingsUiState,
    viewModel: SettingsViewModel,
) {
    if (uiState.isZygiskSupported) {
        item {
            SwitchButton(
                checked = uiState.zygiskEnabled,
                onCheckedChange = viewModel::toggleZygisk,
                label = { Text(stringResource(CoreR.string.zygisk)) },
                secondaryLabel = {
                    Text(
                        stringResource(
                            if (uiState.isZygiskPendingReboot) CoreR.string.reboot_apply_change
                            else CoreR.string.settings_zygisk_summary
                        )
                    )
                },
                modifier = Modifier.fillMaxWidth(),
            )
        }
        item {
            // The switch enforces the DenyList, the rest of the button opens it
            SplitSwitchButton(
                checked = uiState.denyListEnabled,
                onCheckedChange = viewModel::toggleDenyList,
                toggleContentDescription = stringResource(CoreR.string.settings_denylist_title),
                onContainerClick = viewModel::navigateToDenyList,
                label = { Text(stringResource(CoreR.string.denylist)) },
                secondaryLabel = {
                    Text(stringResource(CoreR.string.settings_denylist_config_title))
                },
                modifier = Modifier.fillMaxWidth(),
            )
        }
    }
    item {
        FilledTonalButton(
            onClick = viewModel::createHosts,
            label = { Text(stringResource(CoreR.string.settings_hosts_title)) },
            secondaryLabel = { Text(stringResource(CoreR.string.settings_hosts_summary)) },
            modifier = Modifier.fillMaxWidth(),
        )
    }
}

private fun TransformingLazyColumnScope.superuserItems(
    uiState: SettingsUiState,
    viewModel: SettingsViewModel,
) {
    if (uiState.isSuRestrictSupported) {
        item {
            SwitchButton(
                checked = uiState.suRestrictEnabled,
                onCheckedChange = viewModel::setSuRestrict,
                label = { Text(stringResource(CoreR.string.settings_su_restrict_title)) },
                // What new grants get
                secondaryLabel = {
                    Text(
                        stringResource(
                            if (uiState.suRestrictEnabled) CoreR.string.wear_root_restricted
                            else CoreR.string.wear_root_full
                        )
                    )
                },
                modifier = Modifier.fillMaxWidth(),
            )
        }
    }
    item {
        ChoicePicker(
            title = stringResource(CoreR.string.superuser_notification),
            items = stringArrayResource(CoreR.array.su_notification).asList(),
            selected = uiState.selectedNotificationModeIndex,
            onSelect = viewModel::setNotificationMode,
        )
    }
    item {
        ChoicePicker(
            title = stringResource(CoreR.string.mount_namespace_mode),
            items = stringArrayResource(CoreR.array.namespace).asList(),
            descriptions = stringArrayResource(CoreR.array.namespace_summary).asList(),
            selected = uiState.selectedNamespaceModeIndex,
            onSelect = viewModel::setNamespaceMode,
        )
    }
}

/**
 * The update channel setting: shows [current] and lets the user pick another channel, which
 * is passed to [onSelect]. Indices are those of the update_channel array (Config.updateChannel).
 */
@Composable
fun ChannelPicker(current: Int, onSelect: (Int) -> Unit) {
    ChoicePicker(
        title = stringResource(CoreR.string.settings_update_channel_title),
        items = stringArrayResource(CoreR.array.update_channel).asList(),
        selected = current,
        onSelect = onSelect,
    )
}

/**
 * A setting with one of several values, the Wear counterpart of the phone app's dropdown:
 * a button showing the selected item, which opens a list to pick from.
 */
@Composable
private fun ChoicePicker(
    title: String,
    items: List<String>,
    selected: Int,
    onSelect: (Int) -> Unit,
    descriptions: List<String>? = null,
) {
    var open by rememberSaveable { mutableStateOf(false) }
    val selectedItem = items.getOrNull(selected)

    FilledTonalButton(
        onClick = { open = true },
        label = { Text(title) },
        secondaryLabel = if (selectedItem != null) {
            { Text(selectedItem) }
        } else null,
        modifier = Modifier.fillMaxWidth(),
    )

    Dialog(visible = open, onDismissRequest = { open = false }) {
        val state = rememberTransformingLazyColumnState()
        ScreenScaffold(state) { contentPadding ->
            TransformingLazyColumn(state = state, contentPadding = contentPadding) {
                item {
                    ListHeader { Text(title, textAlign = TextAlign.Center) }
                }
                items.forEachIndexed { index, choice ->
                    val description = descriptions?.getOrNull(index)
                    item(key = index) {
                        RadioButton(
                            selected = index == selected,
                            onSelect = {
                                open = false
                                if (index != selected) onSelect(index)
                            },
                            label = { Text(choice) },
                            secondaryLabel = if (description != null) {
                                { Text(description) }
                            } else null,
                            modifier = Modifier.fillMaxWidth(),
                        )
                    }
                }
            }
        }
    }
}

/**
 * The custom channel URL dialog, shown while the view model asks for it, like the phone app.
 * The URL is typed with the system keyboard of the watch through RemoteInput.
 */
@Composable
private fun CustomChannelUrlDialog(
    dialog: SettingsDialog.CustomChannelUrl?,
    onSave: (String) -> Unit,
    onDismiss: () -> Unit,
) {
    val context = LocalContext.current
    var url by rememberSaveable { mutableStateOf("") }
    // Whether url holds the text of the dialog being shown, so a recreated activity keeps
    // what was typed instead of starting over from the saved URL
    var editing by rememberSaveable { mutableStateOf(false) }

    LaunchedEffect(dialog) {
        if (dialog == null) {
            // Keep url, so the text stays while the dialog animates out
            editing = false
        } else if (!editing) {
            url = dialog.initialUrl
            editing = true
        }
    }

    // Same rule as the phone app: blank resets to no custom channel
    val isValidUrl = url.isBlank() || (
        (url.startsWith("http://", ignoreCase = true) ||
            url.startsWith("https://", ignoreCase = true)) &&
            URLUtil.isValidUrl(url)
        )

    val keyboard = rememberLauncherForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        val text = result.data
            ?.let { RemoteInput.getResultsFromIntent(it) }
            ?.getCharSequence(URL_INPUT_KEY)
        if (text != null) url = text.toString().trim()
    }
    val inputLabel = stringResource(CoreR.string.settings_update_custom_msg)
    val openKeyboard: () -> Unit = {
        val input = RemoteInput.Builder(URL_INPUT_KEY).setLabel(inputLabel).build()
        val intent = RemoteInputIntentHelper.createActionRemoteInputIntent()
        RemoteInputIntentHelper.putRemoteInputsExtra(intent, listOf(input))
        RemoteInputIntentHelper.putTitleExtra(intent, inputLabel)
        // Every Wear OS device has a keyboard, but don't crash on one that doesn't
        try {
            keyboard.launch(intent)
        } catch (e: ActivityNotFoundException) {
            context.toast(CoreR.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }

    AlertDialog(
        visible = dialog != null,
        onDismissRequest = onDismiss,
        title = { Text(inputLabel) },
        text = {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(4.dp),
                modifier = Modifier.fillMaxWidth(),
            ) {
                if (url.isNotBlank()) {
                    Text(
                        text = url,
                        textAlign = TextAlign.Center,
                        color = if (isValidUrl) MaterialTheme.colorScheme.onSurface
                        else MaterialTheme.colorScheme.error,
                    )
                }
                CompactButton(
                    onClick = openKeyboard,
                    icon = { Icon(Icons.Filled.Edit, contentDescription = inputLabel) },
                    colors = ButtonDefaults.filledTonalButtonColors(),
                )
            }
        },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                // An invalid URL stays shown in the error color until it is fixed
                if (isValidUrl) {
                    onSave(url)
                } else {
                    context.toast(CoreR.string.invalid_update_channel, Toast.LENGTH_SHORT)
                }
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = onDismiss)
        },
    )
}

/**
 * Returns a function that runs [onGranted] once the notification permission is granted,
 * asking for it first when needed, like the phone app.
 */
@Composable
private fun rememberNotificationPermissionLauncher(onGranted: () -> Unit): () -> Unit {
    val context = LocalContext.current
    val currentOnGranted by rememberUpdatedState(onGranted)

    val launcher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) {
            currentOnGranted()
        } else {
            context.toast(CoreR.string.post_notifications_denied, Toast.LENGTH_SHORT)
        }
    }

    return remember(launcher, context) {
        {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU ||
                ContextCompat.checkSelfPermission(context, POST_NOTIFICATIONS) ==
                PackageManager.PERMISSION_GRANTED
            ) {
                currentOnGranted()
            } else {
                launcher.launch(POST_NOTIFICATIONS)
            }
        }
    }
}
