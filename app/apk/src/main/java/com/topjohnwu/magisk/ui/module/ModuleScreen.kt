package com.topjohnwu.magisk.ui.module

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Switch
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun ModuleScreen(viewModel: ModuleViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val scrollBehavior = MiuixScrollBehavior()

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.modules),
                scrollBehavior = scrollBehavior
            )
        }
    ) { padding ->
        if (uiState.loading) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                CircularProgressIndicator()
            }
            return@Scaffold
        }

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(horizontal = 12.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }

            item {
                TextButton(
                    text = stringResource(CoreR.string.module_action_install_external),
                    onClick = { viewModel.installPressed() },
                    modifier = Modifier.fillMaxWidth()
                )
            }

            if (uiState.modules.isEmpty()) {
                item {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(vertical = 32.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = stringResource(CoreR.string.module_empty),
                            style = MiuixTheme.textStyles.body1,
                            color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                        )
                    }
                }
            } else {
                items(uiState.modules, key = { it.module.id }) { item ->
                    ModuleCard(item = item, viewModel = viewModel)
                }
            }

            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun ModuleCard(item: ModuleItem, viewModel: ModuleViewModel) {
    val cardAlpha = if (!item.isRemoved && item.isEnabled && !item.showNotice) 1f else 0.5f
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .alpha(cardAlpha)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = item.module.name,
                        style = MiuixTheme.textStyles.body1,
                        textDecoration = strikeThrough,
                    )
                    Text(
                        text = stringResource(
                            CoreR.string.module_version_author,
                            item.module.version,
                            item.module.author
                        ),
                        style = MiuixTheme.textStyles.body2,
                        color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                        textDecoration = strikeThrough,
                    )
                }
                Switch(
                    checked = item.isEnabled,
                    onCheckedChange = { viewModel.toggleEnabled(item) }
                )
            }

            if (item.module.description.isNotEmpty()) {
                Spacer(Modifier.height(8.dp))
                Text(
                    text = item.module.description,
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                    textDecoration = strikeThrough,
                    maxLines = 5,
                )
            }

            if (item.showNotice) {
                Spacer(Modifier.height(8.dp))
                Text(
                    text = item.noticeText,
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.primary,
                )
            }

            Spacer(Modifier.height(8.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                if (item.showAction) {
                    TextButton(
                        text = stringResource(CoreR.string.module_action),
                        onClick = { viewModel.runAction(item.module.id, item.module.name) },
                        enabled = item.isEnabled
                    )
                }

                if (item.showUpdate) {
                    TextButton(
                        text = stringResource(CoreR.string.update),
                        onClick = { viewModel.downloadPressed(item.module.updateInfo) },
                        enabled = item.updateReady
                    )
                }

                Spacer(Modifier.weight(1f))

                TextButton(
                    text = stringResource(
                        if (item.isRemoved) CoreR.string.module_state_restore
                        else CoreR.string.module_state_remove
                    ),
                    onClick = { viewModel.toggleRemove(item) },
                    enabled = !item.isUpdated
                )
            }
        }
    }
}
