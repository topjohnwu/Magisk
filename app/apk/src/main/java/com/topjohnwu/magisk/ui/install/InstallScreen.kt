package com.topjohnwu.magisk.ui.install

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Checkbox
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.extra.SuperArrow
import top.yukonga.miuix.kmp.extra.SuperBottomSheet
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Back
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun InstallScreen(viewModel: InstallViewModel, onBack: () -> Unit) {
    val uiState by viewModel.uiState.collectAsState()
    val showBottomSheet = rememberSaveable { mutableStateOf(false) }

    val filePicker = rememberLauncherForActivityResult(ActivityResultContracts.GetContent()) { uri ->
        uri?.let { viewModel.onPatchFileSelected(it) }
    }

    val secondSlotDialog = rememberConfirmDialog()
    val secondSlotTitle = stringResource(android.R.string.dialog_alert_title)
    val secondSlotMsg = stringResource(CoreR.string.install_inactive_slot_msg)

    LaunchedEffect(uiState.requestFilePicker) {
        if (uiState.requestFilePicker) {
            filePicker.launch("*/*")
            viewModel.onFilePickerConsumed()
        }
    }

    LaunchedEffect(uiState.showSecondSlotWarning) {
        if (uiState.showSecondSlotWarning) {
            val result = secondSlotDialog.awaitConfirm(title = secondSlotTitle, content = secondSlotMsg)
            viewModel.onSecondSlotWarningConsumed()
            if (result == ConfirmResult.Confirmed) {
                viewModel.install()
            }
        }
    }

    val scrollBehavior = MiuixScrollBehavior()
    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.install),
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = MiuixIcons.Back,
                            contentDescription = null,
                            tint = MiuixTheme.colorScheme.onBackground
                        )
                    }
                },
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .verticalScroll(rememberScrollState())
                .padding(padding)
                .padding(horizontal = 12.dp)
                .padding(top = 8.dp, bottom = 16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (!viewModel.skipOptions) {
                OptionsCard(uiState = uiState, viewModel = viewModel)
            }

            Card(modifier = Modifier.fillMaxWidth()) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = stringResource(CoreR.string.install_method_title),
                            style = MiuixTheme.textStyles.headline2,
                        )
                        TextButton(
                            text = stringResource(CoreR.string.install_start),
                            onClick = { showBottomSheet.value = true },
                            enabled = uiState.step >= 1 || viewModel.skipOptions
                        )
                    }
                }
            }

            if (uiState.notes.isNotEmpty()) {
                NotesCard(notes = uiState.notes)
            }
        }
    }

    MethodBottomSheet(
        show = showBottomSheet,
        viewModel = viewModel,
        notes = uiState.notes
    )
}

@Composable
private fun MethodBottomSheet(
    show: androidx.compose.runtime.MutableState<Boolean>,
    viewModel: InstallViewModel,
    notes: String
) {
    SuperBottomSheet(
        show = show,
        onDismissRequest = { show.value = false },
        title = stringResource(CoreR.string.install_method_title),
    ) {
        Column(modifier = Modifier.padding(bottom = 16.dp)) {
            if (notes.isNotEmpty()) {
                Text(
                    text = notes,
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                    modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
                )
            }

            SuperArrow(
                title = stringResource(CoreR.string.select_patch_file),
                summary = stringResource(CoreR.string.select_patch_file_summary),
                onClick = {
                    show.value = false
                    viewModel.selectMethod(InstallViewModel.Method.PATCH)
                }
            )

            if (viewModel.isRooted) {
                SuperArrow(
                    title = stringResource(CoreR.string.direct_install),
                    summary = stringResource(CoreR.string.direct_install_summary),
                    onClick = {
                        show.value = false
                        viewModel.selectMethod(InstallViewModel.Method.DIRECT)
                        viewModel.install()
                    }
                )
            }

            if (!viewModel.noSecondSlot) {
                SuperArrow(
                    title = stringResource(CoreR.string.install_inactive_slot),
                    summary = stringResource(CoreR.string.install_inactive_slot_summary),
                    onClick = {
                        show.value = false
                        viewModel.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                    }
                )
            }
        }
    }
}

@Composable
private fun OptionsCard(uiState: InstallViewModel.UiState, viewModel: InstallViewModel) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
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
}

@Composable
private fun NotesCard(notes: String) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = notes,
            style = MiuixTheme.textStyles.body2,
            color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
            modifier = Modifier.padding(16.dp)
        )
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
