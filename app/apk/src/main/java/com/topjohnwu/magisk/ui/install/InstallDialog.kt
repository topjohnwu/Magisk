package com.topjohnwu.magisk.ui.install

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.MagiskDialog
import com.topjohnwu.magisk.ui.component.MarkdownText
import com.topjohnwu.magisk.ui.component.SettingsArrow
import com.topjohnwu.magisk.ui.component.SettingsSwitch
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun InstallDialog(
    show: Boolean,
    onDismiss: () -> Unit,
    installVm: InstallViewModel,
    modifier: Modifier = Modifier
) {
    val installUiState by installVm.uiState.collectAsStateWithLifecycle()
    var showDownloadDialog by rememberSaveable { mutableStateOf(false) }
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
            showDownloadDialog = true
            installVm.onDownloadDialogConsumed()
        }
    }

    if (showDownloadDialog) {
        DownloadComposableDialog(
            onDismiss = { showDownloadDialog = false },
            onConfirm = { url ->
                showDownloadDialog = false
                installVm.onDownloadUrlSelected(url)
            }
        )
    }

    if (show) {
        Dialog(
            onDismissRequest = onDismiss,
            properties = DialogProperties(
                usePlatformDefaultWidth = false,
                decorFitsSystemWindows = false
            )
        ) {
            val scrollState = rememberScrollState()
            Scaffold(
                modifier = modifier.fillMaxSize(),
                topBar = {
                    TopAppBar(
                        title = { Text(stringResource(CoreR.string.install)) },
                        navigationIcon = {
                            IconButton(onClick = onDismiss) {
                                Icon(
                                    imageVector = Icons.Default.Close,
                                    contentDescription = stringResource(android.R.string.cancel)
                                )
                            }
                        },
                        colors = TopAppBarDefaults.topAppBarColors(
                            containerColor = MaterialTheme.colorScheme.surface
                        )
                    )
                }
            ) { innerPadding ->
                Column(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(innerPadding)
                        .verticalScrollbar(scrollState, contentPadding = PaddingValues(vertical = 12.dp))
                        .verticalScroll(scrollState)
                        .padding(horizontal = 16.dp, vertical = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    if (installUiState.notes.isNotEmpty()) {
                        MarkdownText(installUiState.notes)
                    }

                    if (!installVm.skipOptions) {
                        InstallOptionsSection()
                    }

                    Card(
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(20.dp),
                        colors = CardDefaults.cardColors(
                            containerColor = MaterialTheme.colorScheme.surfaceContainerLow
                        ),
                    ) {
                        SettingsArrow(
                            title = stringResource(CoreR.string.select_patch_file),
                            onClick = {
                                onDismiss()
                                installVm.selectMethod(InstallViewModel.Method.PATCH)
                            },
                        )

                        SettingsArrow(
                            title = stringResource(CoreR.string.download_patch_file),
                            onClick = {
                                onDismiss()
                                installVm.selectMethod(InstallViewModel.Method.DOWNLOAD)
                            },
                        )

                        if (installVm.isRooted) {
                            SettingsArrow(
                                title = stringResource(CoreR.string.direct_install),
                                onClick = {
                                    onDismiss()
                                    installVm.selectMethod(InstallViewModel.Method.DIRECT)
                                    installVm.install()
                                },
                            )
                        }

                        if (!installVm.noSecondSlot) {
                            SettingsArrow(
                                title = stringResource(CoreR.string.install_inactive_slot),
                                onClick = {
                                    onDismiss()
                                    installVm.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                                },
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun InstallOptionsSection(
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
    ) {
        if (!Info.isSAR) {
            var keepVerity by remember { mutableStateOf(Config.keepVerity) }
            SettingsSwitch(
                title = stringResource(CoreR.string.keep_dm_verity),
                checked = keepVerity,
                onCheckedChange = {
                    keepVerity = it
                    Config.keepVerity = it
                }
            )
        }
        if (Info.isFDE) {
            var keepEnc by remember { mutableStateOf(Config.keepEnc) }
            SettingsSwitch(
                title = stringResource(CoreR.string.keep_force_encryption),
                checked = keepEnc,
                onCheckedChange = {
                    keepEnc = it
                    Config.keepEnc = it
                }
            )
        }
        if (!Info.ramdisk) {
            var recovery by remember { mutableStateOf(Config.recovery) }
            SettingsSwitch(
                title = stringResource(CoreR.string.recovery_mode),
                checked = recovery,
                onCheckedChange = {
                    recovery = it
                    Config.recovery = it
                }
            )
        }
    }
}

@Composable
fun DownloadComposableDialog(
    onDismiss: () -> Unit,
    onConfirm: (Uri) -> Unit,
    modifier: Modifier = Modifier
) {
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

    val submit = {
        isValidUrl(url.trim())?.let {
            onConfirm(it)
        } ?: run {
            isError = true
        }
    }

    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = stringResource(CoreR.string.download_dialog_title),
        confirmText = stringResource(android.R.string.ok),
        onConfirm = submit,
        dismissText = stringResource(android.R.string.cancel),
        onDismiss = onDismiss,
    ) {
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
                    onDone = { submit() }
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
    }
}
