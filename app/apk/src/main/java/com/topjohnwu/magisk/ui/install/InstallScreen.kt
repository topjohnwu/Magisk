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
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Checkbox
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun InstallScreen(viewModel: InstallViewModel) {
    val uiState by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 12.dp)
            .padding(top = 8.dp, bottom = 16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        if (!viewModel.skipOptions) {
            OptionsCard(uiState = uiState, viewModel = viewModel)
        }

        MethodCard(uiState = uiState, viewModel = viewModel)

        if (uiState.notes.isNotEmpty()) {
            NotesCard(notes = uiState.notes)
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
private fun MethodCard(uiState: InstallViewModel.UiState, viewModel: InstallViewModel) {
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
                if (uiState.step == 1) {
                    TextButton(
                        text = stringResource(CoreR.string.install_start),
                        onClick = { viewModel.install() },
                        enabled = viewModel.canInstall
                    )
                }
            }

            if (uiState.step == 1) {
                Spacer(Modifier.height(8.dp))

                MethodRadioRow(
                    label = stringResource(CoreR.string.select_patch_file),
                    selected = uiState.method == InstallViewModel.Method.PATCH,
                    onClick = { viewModel.selectMethod(InstallViewModel.Method.PATCH) }
                )
                if (viewModel.isRooted) {
                    MethodRadioRow(
                        label = stringResource(CoreR.string.direct_install),
                        selected = uiState.method == InstallViewModel.Method.DIRECT,
                        onClick = { viewModel.selectMethod(InstallViewModel.Method.DIRECT) }
                    )
                }
                if (!viewModel.noSecondSlot) {
                    MethodRadioRow(
                        label = stringResource(CoreR.string.install_inactive_slot),
                        selected = uiState.method == InstallViewModel.Method.INACTIVE_SLOT,
                        onClick = { viewModel.selectMethod(InstallViewModel.Method.INACTIVE_SLOT) }
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

@Composable
private fun MethodRadioRow(label: String, selected: Boolean, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Checkbox(
            checked = selected,
            onCheckedChange = { onClick() }
        )
        Text(
            text = label,
            style = MiuixTheme.textStyles.body1,
        )
    }
}
