package com.topjohnwu.magisk.wear.module

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Undo
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.ButtonDefaults
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.ui.module.ModuleItem
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.component.ScreenTitle
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import com.topjohnwu.magisk.wear.theme.destructiveButtonColors
import com.topjohnwu.magisk.core.R as CoreR

/** The module in directory [dir]. [viewModel] is the one [ModulesScreen] uses. */
@Composable
fun ModuleDetailScreen(
    dir: String,
    viewModel: ModuleViewModel,
    navigator: WearNavigator,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val moduleItem = uiState.modules.firstOrNull { it.module.base.name == dir }

    CollectNavEvents(viewModel, navigator)

    LaunchedEffect(Unit) {
        // E.g. restored after process death straight into this screen
        if (uiState.modules.isEmpty()) viewModel.startLoading()
    }

    // If the module disappears while this screen is shown (e.g. removed by a reload after it was
    // uninstalled elsewhere), leave instead of showing nothing. Only leave if this screen is
    // still on top, so the screen below is never popped.
    val isGone = !uiState.loading && moduleItem == null
    LaunchedEffect(isGone) {
        if (isGone && navigator.current() == WearRoute.ModuleDetail(dir)) navigator.pop()
    }

    val state = rememberTransformingLazyColumnState()

    if (uiState.loading) {
        LoadingScreen()
        return
    }

    if (moduleItem == null) return

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item(key = "header") {
                ModuleHeader(moduleItem)
            }
            item(key = "enabled") {
                SwitchButton(
                    checked = moduleItem.isEnabled,
                    onCheckedChange = { viewModel.toggleEnabled(moduleItem) },
                    label = { Text(stringResource(CoreR.string.wear_module_enabled)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            // Shown when the phone app shows it: the action of a disabled or removed module
            // can't run as intended
            if (moduleItem.showAction && moduleItem.isEnabled && !moduleItem.isRemoved) {
                item(key = "action") {
                    FilledTonalButton(
                        onClick = { viewModel.runAction(dir, moduleItem.module.name) },
                        icon = { Icon(Icons.Filled.PlayArrow, contentDescription = null) },
                        label = { Text(stringResource(CoreR.string.wear_module_run_action)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
            item(key = "remove") {
                Button(
                    onClick = { viewModel.toggleRemove(moduleItem) },
                    // An update waiting for the reboot replaces the module anyway
                    enabled = !moduleItem.isUpdated,
                    colors = if (moduleItem.isRemoved) {
                        ButtonDefaults.filledTonalButtonColors()
                    } else {
                        destructiveButtonColors()
                    },
                    icon = {
                        Icon(
                            if (moduleItem.isRemoved) Icons.AutoMirrored.Filled.Undo else Icons.Filled.Delete,
                            contentDescription = null,
                        )
                    },
                    label = {
                        Text(
                            stringResource(
                                if (moduleItem.isRemoved) CoreR.string.module_state_restore
                                else CoreR.string.wear_module_remove
                            )
                        )
                    },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
private fun ModuleHeader(item: ModuleItem) {
    val module = item.module
    val colors = MaterialTheme.colorScheme
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None

    Column(
        modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        ScreenTitle(text = module.name, textDecoration = strikeThrough)
        Text(
            text = stringResource(CoreR.string.module_version_author, module.version, module.author),
            style = MaterialTheme.typography.bodySmall,
            color = colors.onSurfaceVariant,
            textAlign = TextAlign.Center,
        )
        Text(
            text = stringResource(
                when {
                    item.isRemoved -> CoreR.string.wear_module_removed
                    item.isEnabled -> CoreR.string.wear_module_active
                    else -> CoreR.string.wear_module_disabled
                }
            ),
            style = MaterialTheme.typography.labelSmall,
            color = if (item.isEnabled && !item.isRemoved) colors.primary else colors.onSurfaceVariant,
            modifier = Modifier.padding(top = 2.dp),
        )
        if (item.showNotice) {
            Text(
                text = item.noticeText.getText(LocalResources.current),
                style = MaterialTheme.typography.labelSmall,
                color = colors.primary,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(top = 4.dp),
            )
        }
        if (module.description.isNotBlank()) {
            Text(
                text = module.description,
                style = MaterialTheme.typography.bodySmall,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(top = 6.dp),
            )
        }
    }
}
