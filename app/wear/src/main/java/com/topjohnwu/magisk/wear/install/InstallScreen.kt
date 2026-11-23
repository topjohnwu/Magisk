package com.topjohnwu.magisk.wear.install

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.TransformingLazyColumnScope
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.wear.files.FileKind
import com.topjohnwu.magisk.wear.files.rememberFilePicker
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.core.R as CoreR

/**
 * The ways to install Magisk, like the phone app's install dialog: patch an image file,
 * install directly, or install to the inactive slot after an OTA. Each one opens the flash
 * screen through [InstallViewModel.install].
 */
@Composable
fun InstallScreen(viewModel: InstallViewModel, navigator: WearNavigator) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    var showDirectConfirm by rememberSaveable { mutableStateOf(false) }

    CollectNavEvents(viewModel, navigator)

    // Sets the method itself and goes on to install, see onPatchSelected
    val pickImage = rememberFilePicker(FileKind.IMAGE) { uri ->
        viewModel.onPatchSelected(uri)
    }

    val state = rememberTransformingLazyColumnState()
    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.install)) }
            }
            if (!viewModel.skipOptions) {
                installOptions()
            }
            item {
                FilledTonalButton(
                    onClick = pickImage,
                    label = { Text(stringResource(CoreR.string.patch_image)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            if (viewModel.isRooted) {
                item {
                    Button(
                        onClick = { showDirectConfirm = true },
                        label = { Text(stringResource(CoreR.string.direct_install)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
            // Only offered with root on an A/B device, like the phone app
            if (!viewModel.noSecondSlot) {
                item {
                    FilledTonalButton(
                        onClick = { viewModel.selectMethod(InstallViewModel.Method.INACTIVE_SLOT) },
                        label = { Text(stringResource(CoreR.string.install_inactive_slot)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
        }
    }

    AlertDialog(
        visible = showDirectConfirm,
        onDismissRequest = { showDirectConfirm = false },
        title = { Text(stringResource(CoreR.string.direct_install)) },
        text = { Text(stringResource(CoreR.string.wear_install_direct_msg)) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                showDirectConfirm = false
                // Set only now: the method is kept in memory, and the dialog survives the process
                viewModel.selectMethod(InstallViewModel.Method.DIRECT)
                viewModel.install()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { showDirectConfirm = false })
        },
    )

    // Shown while the view model asks for it, so it never outlives the method it confirms
    AlertDialog(
        visible = uiState.showSecondSlotWarning,
        onDismissRequest = { viewModel.onSecondSlotWarningConsumed() },
        title = { Text(stringResource(android.R.string.dialog_alert_title)) },
        text = { Text(stringResource(CoreR.string.install_inactive_slot_msg)) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                viewModel.onSecondSlotWarningConsumed()
                viewModel.install()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { viewModel.onSecondSlotWarningConsumed() })
        },
    )
}

/** The patching options of the phone app, each shown only where it applies. */
private fun TransformingLazyColumnScope.installOptions() {
    if (!Info.isSAR) {
        item {
            ConfigSwitch(CoreR.string.keep_dm_verity, Config.keepVerity) { Config.keepVerity = it }
        }
    }
    if (Info.isFDE) {
        item {
            ConfigSwitch(CoreR.string.keep_force_encryption, Config.keepEnc) { Config.keepEnc = it }
        }
    }
    if (!Info.ramdisk) {
        item {
            ConfigSwitch(CoreR.string.recovery_mode, Config.recovery) { Config.recovery = it }
        }
    }
}

@Composable
private fun ConfigSwitch(label: Int, initial: Boolean, onChange: (Boolean) -> Unit) {
    // Config is not observable, so keep what the switch shows here, like the phone app
    var checked by remember { mutableStateOf(initial) }
    SwitchButton(
        checked = checked,
        onCheckedChange = {
            checked = it
            onChange(it)
        },
        label = { Text(stringResource(label)) },
        modifier = Modifier.fillMaxWidth(),
    )
}
