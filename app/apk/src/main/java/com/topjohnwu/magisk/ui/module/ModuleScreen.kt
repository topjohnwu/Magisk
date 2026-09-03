package com.topjohnwu.magisk.ui.module

import android.provider.OpenableColumns
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.animateContentSize
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.spring
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
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
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Undo
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.CloudUpload
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.MagiskDialog
import com.topjohnwu.magisk.ui.component.MarkdownTextAsync
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.utils.textHolder
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ModuleScreen(
    viewModel: ModuleViewModel,
    modifier: Modifier = Modifier
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val colorScheme = MaterialTheme.colorScheme
    val context = LocalContext.current
    val resources = LocalResources.current
    val scope = rememberCoroutineScope()

    val localInstallDialog = rememberConfirmDialog()
    val confirmInstallTitle = stringResource(CoreR.string.confirm_install_title)

    var pendingOnlineModule by remember { mutableStateOf<OnlineModule?>(null) }
    var showOnlineDialog by rememberSaveable { mutableStateOf(false) }

    val filePicker = rememberLauncherForActivityResult(ActivityResultContracts.GetContent()) { uri ->
        if (uri != null) {
            val displayName = context.contentResolver.query(uri, null, null, null, null)?.use { cursor ->
                val idx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                if (cursor.moveToFirst() && idx >= 0) cursor.getString(idx) else null
            } ?: uri.lastPathSegment ?: "module.zip"
            scope.launch {
                val result = localInstallDialog.awaitConfirm(
                    title = confirmInstallTitle,
                    content = resources.getString(CoreR.string.confirm_install, displayName),
                )
                if (result == ConfirmResult.Confirmed) {
                    viewModel.confirmLocalInstall(uri)
                }
            }
        }
    }

    if (showOnlineDialog && pendingOnlineModule != null) {
        OnlineModuleDialog(
            item = pendingOnlineModule!!,
            onDownload = { install ->
                showOnlineDialog = false
                (context as? MainActivity)?.let { activity ->
                    DownloadEngine.startWithActivity(
                        activity,
                        OnlineModuleSubject(pendingOnlineModule!!, install)
                    )
                }
                pendingOnlineModule = null
            },
            onDismiss = {
                showOnlineDialog = false
                pendingOnlineModule = null
            }
        )
    }

    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.modules)) },
                scrollBehavior = scrollBehavior
            )
        },
        floatingActionButton = {
            FloatingActionButton(
                onClick = { filePicker.launch("application/zip") },
                containerColor = MaterialTheme.colorScheme.primaryContainer,
                contentColor = MaterialTheme.colorScheme.onPrimaryContainer,
                content = {
                    Icon(
                        imageVector = Icons.Default.Add,
                        contentDescription = stringResource(CoreR.string.module_action_install_external),
                        modifier = Modifier.size(28.dp),
                    )
                },
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

        if (uiState.modules.isEmpty()) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.spacedBy(12.dp),
                    modifier = Modifier.padding(horizontal = 32.dp)
                ) {
                    Icon(
                        painter = painterResource(R.drawable.ic_module),
                        contentDescription = null,
                        tint = colorScheme.onSurfaceVariant.copy(alpha = 0.6f),
                        modifier = Modifier.size(64.dp)
                    )
                    Text(
                        text = stringResource(CoreR.string.module_empty),
                        style = MaterialTheme.typography.bodyLarge,
                        color = colorScheme.onSurfaceVariant,
                        textAlign = TextAlign.Center
                    )
                }
            }
            return@Scaffold
        }

        val listState = rememberLazyListState()
        LazyColumn(
            state = listState,
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .verticalScrollbar(listState, contentPadding = PaddingValues(top = 4.dp, bottom = 80.dp))
                .padding(horizontal = 12.dp),
            contentPadding = PaddingValues(top = 4.dp, bottom = 80.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }
            items(
                items = uiState.modules,
                key = { it.module.id },
                contentType = { "ModuleCard" }
            ) { item ->
                ModuleCard(
                    item = item,
                    viewModel = viewModel,
                    onUpdateClick = { onlineModule ->
                        if (onlineModule != null && Info.isConnected.value == true) {
                            pendingOnlineModule = onlineModule
                            showOnlineDialog = true
                        }
                    }
                )
            }
            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun ModuleCard(
    item: ModuleItem,
    viewModel: ModuleViewModel,
    onUpdateClick: (OnlineModule?) -> Unit,
    modifier: Modifier = Modifier
) {
    val infoAlpha = if (!item.isRemoved && item.isEnabled && !item.showNotice) 1f else 0.5f
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None
    val colorScheme = MaterialTheme.colorScheme
    var expanded by rememberSaveable(item.module.id) { mutableStateOf(false) }
    val hasDescription = item.module.description.isNotBlank()

    Card(
        onClick = { expanded = !expanded },
        enabled = hasDescription,
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = colorScheme.surfaceContainerLow)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Column(modifier = Modifier.alpha(infoAlpha)) {
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
                            style = MaterialTheme.typography.titleMedium,
                            textDecoration = strikeThrough,
                        )
                        Spacer(Modifier.height(2.dp))
                        Text(
                            text = stringResource(
                                CoreR.string.module_version_author,
                                item.module.version,
                                item.module.author
                            ),
                            style = MaterialTheme.typography.bodyMedium,
                            color = colorScheme.onSurfaceVariant,
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
                            .padding(top = 6.dp)
                            .animateContentSize(
                                animationSpec = spring(
                                    dampingRatio = Spring.DampingRatioLowBouncy,
                                    stiffness = Spring.StiffnessMedium
                                )
                            )
                    ) {
                        Text(
                            text = item.module.description,
                            style = MaterialTheme.typography.bodyMedium,
                            color = colorScheme.onSurfaceVariant,
                            textDecoration = strikeThrough,
                            overflow = if (expanded) TextOverflow.Clip else TextOverflow.Ellipsis,
                            maxLines = if (expanded) Int.MAX_VALUE else 3,
                        )
                    }
                }

                if (item.showNotice) {
                    Spacer(Modifier.height(6.dp))
                    Text(
                        text = textHolder(item.noticeText),
                        style = MaterialTheme.typography.bodyMedium,
                        fontWeight = FontWeight.Medium,
                        color = colorScheme.primary,
                    )
                }
            }

            HorizontalDivider(modifier = Modifier.padding(vertical = 12.dp))

            Row(verticalAlignment = Alignment.CenterVertically) {
                AnimatedVisibility(
                    visible = item.isEnabled && !item.isRemoved,
                    enter = fadeIn(),
                    exit = fadeOut()
                ) {
                    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        if (item.showAction) {
                            FilledTonalButton(
                                contentPadding = PaddingValues(horizontal = 14.dp, vertical = 8.dp),
                                onClick = { viewModel.runAction(item.module.id, item.module.name) },
                            ) {
                                Row(
                                    verticalAlignment = Alignment.CenterVertically,
                                    horizontalArrangement = Arrangement.spacedBy(4.dp),
                                ) {
                                    Icon(
                                        modifier = Modifier.size(18.dp),
                                        imageVector = Icons.Default.PlayArrow,
                                        contentDescription = stringResource(CoreR.string.module_action)
                                    )
                                    Text(
                                        text = stringResource(CoreR.string.module_action),
                                        style = MaterialTheme.typography.labelLarge,
                                    )
                                }
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
                    FilledTonalButton(
                        modifier = Modifier.padding(end = 8.dp),
                        colors = ButtonDefaults.filledTonalButtonColors(
                            containerColor = colorScheme.tertiaryContainer,
                            contentColor = colorScheme.onTertiaryContainer
                        ),
                        contentPadding = PaddingValues(horizontal = 14.dp, vertical = 8.dp),
                        onClick = { onUpdateClick(item.module.updateInfo) },
                    ) {
                        Row(
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(4.dp),
                        ) {
                            Icon(
                                modifier = Modifier.size(18.dp),
                                imageVector = Icons.Default.CloudUpload,
                                contentDescription = stringResource(CoreR.string.update),
                            )
                            Text(
                                text = stringResource(CoreR.string.update),
                                style = MaterialTheme.typography.labelLarge,
                            )
                        }
                    }
                }

                FilledTonalButton(
                    colors = if (item.isRemoved) {
                        ButtonDefaults.filledTonalButtonColors()
                    } else {
                        ButtonDefaults.filledTonalButtonColors(
                            containerColor = colorScheme.errorContainer,
                            contentColor = colorScheme.onErrorContainer
                        )
                    },
                    contentPadding = PaddingValues(horizontal = 14.dp, vertical = 8.dp),
                    onClick = { viewModel.toggleRemove(item) },
                    enabled = !item.isUpdated
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(4.dp),
                    ) {
                        Icon(
                            modifier = Modifier.size(18.dp),
                            imageVector = if (item.isRemoved) Icons.AutoMirrored.Filled.Undo else Icons.Default.Delete,
                            contentDescription = null
                        )
                        Text(
                            text = stringResource(
                                if (item.isRemoved) CoreR.string.module_state_restore
                                else CoreR.string.module_state_remove
                            ),
                            style = MaterialTheme.typography.labelLarge,
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun OnlineModuleDialog(
    item: OnlineModule,
    onDownload: (install: Boolean) -> Unit,
    onDismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    val svc = ServiceLocator.networkService
    val title = stringResource(
        CoreR.string.repo_install_title,
        item.name, item.version, item.versionCode
    )

    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = title,
        confirmText = stringResource(CoreR.string.install),
        onConfirm = { onDownload(true) },
        dismissText = stringResource(android.R.string.cancel),
        onDismiss = onDismiss,
        neutralText = stringResource(CoreR.string.download),
        onNeutral = { onDownload(false) },
    ) {
        MarkdownTextAsync {
            val str = svc.fetchString(item.changelog)
            if (str.length > 1000) str.substring(0, 1000) else str
        }
    }
}
