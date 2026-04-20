package com.topjohnwu.magisk.ui.install

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Checkbox
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.MarkdownText
import com.topjohnwu.magisk.ui.component.SettingsArrow
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun InstallBottomSheet(
    show: MutableState<Boolean>,
    installVm: InstallViewModel,
) {
    val installUiState by installVm.uiState.collectAsState()
    val showDownloadDialog = rememberSaveable { mutableStateOf(false) }
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

    LaunchedEffect(installUiState.showDownloadDialog) {
        if (installUiState.showDownloadDialog) {
            showDownloadDialog.value = true
            installVm.onDownloadDialogConsumed()
        }
    }

    if (showDownloadDialog.value) {
        DownloadComposableDialog(
            showDialog = showDownloadDialog,
            onConfirm = { url -> installVm.onDownloadUrlSelected(url) }
        )
    }

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
                )

                SettingsArrow(
                    title = stringResource(CoreR.string.download_patch_file),
                    onClick = {
                        show.value = false
                        installVm.selectMethod(InstallViewModel.Method.DOWNLOAD)
                    },
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
fun DownloadComposableDialog(
    showDialog: MutableState<Boolean>,
    onConfirm: (Uri) -> Unit
) {
    if (!showDialog.value) return

    var url by rememberSaveable { mutableStateOf("") }
    var isError by rememberSaveable { mutableStateOf(false) }

    fun isValidUrl(url: String): Uri? {
        if (url.isEmpty()) return null
        val uri = url.toUri()
        if (!uri.scheme.equals("https", ignoreCase = true)) return null
        if (uri.host.isNullOrEmpty()) return null
        if (uri.path.isNullOrEmpty()) return null
        return uri
    }

    AlertDialog(
        onDismissRequest = { showDialog.value = false },
        title = { Text(stringResource(CoreR.string.download_dialog_title)) },
        text = {
            Column(modifier = Modifier.padding(top = 8.dp)) {
                OutlinedTextField(
                    value = url,
                    onValueChange = {
                        url = it
                        isError = false
                    },
                    modifier = Modifier.fillMaxWidth(),
                    label = { Text(stringResource(CoreR.string.download_dialog_msg)) },
                    isError = isError,
                    singleLine = true,
                    keyboardOptions = KeyboardOptions(
                        keyboardType = KeyboardType.Uri,
                        imeAction = ImeAction.Done
                    ),
                    keyboardActions = KeyboardActions(
                        onDone = {
                            isValidUrl(url.trim())?.let {
                                showDialog.value = false
                                onConfirm(it)
                            } ?: run {
                                isError = true
                            }
                        }
                    )
                )
                if (isError) {
                    Text(
                        text = stringResource(CoreR.string.download_dialog_title),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall,
                        modifier = Modifier.padding(start = 16.dp, top = 4.dp)
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    isValidUrl(url.trim())?.let {
                        showDialog.value = false
                        onConfirm(it)
                    } ?: run {
                        isError = true
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
