package com.topjohnwu.magisk.ui.module

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.animateContentSize
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.FloatingActionButton
import top.yukonga.miuix.kmp.basic.HorizontalDivider
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Switch
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Add
import top.yukonga.miuix.kmp.icon.extended.Delete
import top.yukonga.miuix.kmp.icon.extended.Play
import top.yukonga.miuix.kmp.icon.extended.Undo
import top.yukonga.miuix.kmp.icon.extended.UploadCloud
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun ModuleScreen(viewModel: ModuleViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val scrollBehavior = MiuixScrollBehavior()
    val colorScheme = MiuixTheme.colorScheme

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.modules),
                scrollBehavior = scrollBehavior
            )
        },
        floatingActionButton = {
            FloatingActionButton(
                onClick = { viewModel.installPressed() },
                shadowElevation = 0.dp,
                modifier = Modifier
                    .padding(bottom = 88.dp, end = 20.dp)
                    .border(0.05.dp, colorScheme.outline.copy(alpha = 0.5f), CircleShape),
                content = {
                    Icon(
                        imageVector = MiuixIcons.Add,
                        contentDescription = stringResource(CoreR.string.module_action_install_external),
                        modifier = Modifier.size(28.dp),
                        tint = colorScheme.onPrimary
                    )
                },
            )
        },
        popupHost = { }
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

        if (uiState.modules.isEmpty()) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.module_empty),
                    style = MiuixTheme.textStyles.body1,
                    color = colorScheme.onSurfaceVariantSummary
                )
            }
            return@Scaffold
        }

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .padding(horizontal = 12.dp),
            contentPadding = PaddingValues(bottom = 88.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }
            items(uiState.modules, key = { it.module.id }) { item ->
                ModuleCard(item = item, viewModel = viewModel)
            }
            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun ModuleCard(item: ModuleItem, viewModel: ModuleViewModel) {
    val cardAlpha = if (!item.isRemoved && item.isEnabled && !item.showNotice) 1f else 0.5f
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None
    val colorScheme = MiuixTheme.colorScheme
    val actionIconTint = colorScheme.onSurface.copy(alpha = 0.8f)
    val actionBg = colorScheme.secondaryContainer.copy(alpha = 0.8f)
    val updateBg = colorScheme.tertiaryContainer.copy(alpha = 0.6f)
    val updateTint = colorScheme.onTertiaryContainer.copy(alpha = 0.8f)
    var expanded by rememberSaveable(item.module.id) { mutableStateOf(false) }
    val hasDescription = item.module.description.isNotBlank()

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .alpha(cardAlpha),
        insideMargin = PaddingValues(16.dp),
        onClick = { if (hasDescription) expanded = !expanded }
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(
                modifier = Modifier
                    .weight(1f)
                    .padding(end = 4.dp)
            ) {
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
                    color = colorScheme.onSurfaceVariantSummary,
                    textDecoration = strikeThrough,
                )
            }
            Switch(
                checked = item.isEnabled,
                onCheckedChange = { viewModel.toggleEnabled(item) }
            )
        }

        if (hasDescription) {
            Box(
                modifier = Modifier
                    .padding(top = 2.dp)
                    .animateContentSize()
            ) {
                Text(
                    text = item.module.description,
                    style = MiuixTheme.textStyles.body2,
                    color = colorScheme.onSurfaceVariantSummary,
                    textDecoration = strikeThrough,
                    overflow = if (expanded) TextOverflow.Clip else TextOverflow.Ellipsis,
                    maxLines = if (expanded) Int.MAX_VALUE else 4,
                )
            }
        }

        if (item.showNotice) {
            Spacer(Modifier.height(4.dp))
            Text(
                text = item.noticeText,
                style = MiuixTheme.textStyles.body2,
                color = colorScheme.primary,
            )
        }

        HorizontalDivider(
            modifier = Modifier.padding(vertical = 8.dp),
            thickness = 0.5.dp,
            color = colorScheme.outline.copy(alpha = 0.5f)
        )

        Row(verticalAlignment = Alignment.CenterVertically) {
            AnimatedVisibility(
                visible = item.isEnabled && !item.isRemoved,
                enter = fadeIn(),
                exit = fadeOut()
            ) {
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    if (item.showAction) {
                        IconButton(
                            backgroundColor = actionBg,
                            minHeight = 35.dp,
                            minWidth = 35.dp,
                            onClick = { viewModel.runAction(item.module.id, item.module.name) },
                        ) {
                            Icon(
                                modifier = Modifier.size(20.dp),
                                imageVector = MiuixIcons.Play,
                                tint = actionIconTint,
                                contentDescription = stringResource(CoreR.string.module_action)
                            )
                        }
                    }
                }
            }

            Spacer(Modifier.weight(1f))

            AnimatedVisibility(
                visible = item.showUpdate && item.updateReady,
                enter = fadeIn(),
                exit = fadeOut()
            ) {
                IconButton(
                    modifier = Modifier.padding(end = 8.dp),
                    backgroundColor = updateBg,
                    minHeight = 35.dp,
                    minWidth = 35.dp,
                    onClick = { viewModel.downloadPressed(item.module.updateInfo) },
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 10.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(4.dp),
                    ) {
                        Icon(
                            modifier = Modifier.size(20.dp),
                            imageVector = MiuixIcons.UploadCloud,
                            tint = updateTint,
                            contentDescription = stringResource(CoreR.string.update),
                        )
                        Text(
                            text = stringResource(CoreR.string.update),
                            color = updateTint,
                            style = MiuixTheme.textStyles.body2,
                        )
                    }
                }
            }

            IconButton(
                backgroundColor = actionBg,
                minHeight = 35.dp,
                minWidth = 35.dp,
                onClick = { viewModel.toggleRemove(item) },
                enabled = !item.isUpdated
            ) {
                Row(
                    modifier = Modifier.padding(horizontal = 10.dp),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(4.dp),
                ) {
                    Icon(
                        modifier = Modifier.size(20.dp),
                        imageVector = if (item.isRemoved) MiuixIcons.Undo else MiuixIcons.Delete,
                        tint = actionIconTint,
                        contentDescription = null
                    )
                    Text(
                        text = stringResource(
                            if (item.isRemoved) CoreR.string.module_state_restore
                            else CoreR.string.module_state_remove
                        ),
                        color = actionIconTint,
                        style = MiuixTheme.textStyles.body2,
                    )
                }
            }
        }
    }
}
