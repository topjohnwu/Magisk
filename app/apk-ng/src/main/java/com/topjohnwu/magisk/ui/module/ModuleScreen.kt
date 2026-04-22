package com.topjohnwu.magisk.ui.module

import android.provider.OpenableColumns
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.animateContentSize
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
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
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Undo
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.CloudUpload
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.MarkdownTextAsync
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.utils.textHolder
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ModuleScreen(viewModel: ModuleViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val colorScheme = MaterialTheme.colorScheme
    val context = LocalContext.current
    val resources = LocalResources.current
    val scope = rememberCoroutineScope()
    val activity = context as MainActivity

    val localInstallDialog = rememberConfirmDialog()
    val confirmInstallTitle = stringResource(CoreR.string.confirm_install_title)

    var pendingOnlineModule by remember { mutableStateOf<OnlineModule?>(null) }
    val showOnlineDialog = rememberSaveable { mutableStateOf(false) }

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

    if (showOnlineDialog.value && pendingOnlineModule != null) {
        OnlineModuleDialog(
            item = pendingOnlineModule!!,
            showDialog = showOnlineDialog,
            onDownload = { install ->
                showOnlineDialog.value = false
                DownloadEngine.startWithActivity(
                    activity,
                    OnlineModuleSubject(pendingOnlineModule!!, install)
                )
                pendingOnlineModule = null
            },
            onDismiss = {
                showOnlineDialog.value = false
                pendingOnlineModule = null
            }
        )
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.modules)) },
                scrollBehavior = scrollBehavior
            )
        },
        floatingActionButton = {
            FloatingActionButton(
                onClick = { filePicker.launch("application/zip") },
                shape = CircleShape,
                modifier = Modifier
                    .padding(bottom = 88.dp, end = 20.dp)
                    .border(0.05.dp, colorScheme.outline.copy(alpha = 0.5f), CircleShape),
                content = {
                    Icon(
                        imageVector = Icons.Default.Add,
                        contentDescription = stringResource(CoreR.string.module_action_install_external),
                        modifier = Modifier.size(28.dp),
                        tint = colorScheme.onPrimaryContainer
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
                Text(
                    text = stringResource(CoreR.string.module_empty),
                    style = MaterialTheme.typography.bodyLarge,
                    color = colorScheme.onSurfaceVariant
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
            contentPadding = PaddingValues(bottom = 160.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }
            items(uiState.modules, key = { it.module.id }) { item ->
                ModuleCard(
                    item = item,
                    viewModel = viewModel,
                    onUpdateClick = { onlineModule ->
                        if (onlineModule != null && Info.isConnected.value == true) {
                            pendingOnlineModule = onlineModule
                            showOnlineDialog.value = true
                        }
                    }
                )
            }
            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun ModuleCard(item: ModuleItem, viewModel: ModuleViewModel, onUpdateClick: (OnlineModule?) -> Unit) {
    val infoAlpha = if (!item.isRemoved && item.isEnabled && !item.showNotice) 1f else 0.5f
    val strikeThrough = if (item.isRemoved) TextDecoration.LineThrough else TextDecoration.None
    val colorScheme = MaterialTheme.colorScheme
    val actionIconTint = colorScheme.onSurface.copy(alpha = 0.8f)
    val actionBg = colorScheme.secondaryContainer.copy(alpha = 0.8f)
    val updateBg = colorScheme.tertiaryContainer.copy(alpha = 0.6f)
    val updateTint = colorScheme.onTertiaryContainer.copy(alpha = 0.8f)
    val removeBg = colorScheme.errorContainer.copy(alpha = 0.6f)
    val removeTint = colorScheme.onErrorContainer.copy(alpha = 0.8f)
    var expanded by rememberSaveable(item.module.id) { mutableStateOf(false) }
    val hasDescription = item.module.description.isNotBlank()

    Card(
        modifier = Modifier.fillMaxWidth().clickable(enabled = hasDescription) { expanded = !expanded },
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
                        style = MaterialTheme.typography.bodyLarge,
                        textDecoration = strikeThrough,
                    )
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
                        .padding(top = 2.dp)
                        .animateContentSize()
                ) {
                    Text(
                        text = item.module.description,
                        style = MaterialTheme.typography.bodyMedium,
                        color = colorScheme.onSurfaceVariant,
                        textDecoration = strikeThrough,
                        overflow = if (expanded) TextOverflow.Clip else TextOverflow.Ellipsis,
                        maxLines = if (expanded) Int.MAX_VALUE else 4,
                    )
                }
            }

            if (item.showNotice) {
                Spacer(Modifier.height(4.dp))
                Text(
                    text = textHolder(item.noticeText),
                    style = MaterialTheme.typography.bodyMedium,
                    color = colorScheme.primary,
                )
            }
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
                        Button(
                            colors = ButtonDefaults.buttonColors(containerColor = actionBg),
                            contentPadding = PaddingValues(horizontal = 10.dp),
                            onClick = { viewModel.runAction(item.module.id, item.module.name) },
                        ) {
                            Row(
                                verticalAlignment = Alignment.CenterVertically,
                                horizontalArrangement = Arrangement.spacedBy(4.dp),
                            ) {
                                Icon(
                                    modifier = Modifier.size(20.dp),
                                    imageVector = Icons.Default.PlayArrow,
                                    tint = actionIconTint,
                                    contentDescription = stringResource(CoreR.string.module_action)
                                )
                                Text(
                                    text = stringResource(CoreR.string.module_action),
                                    color = actionIconTint,
                                    style = MaterialTheme.typography.bodyMedium,
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
                Button(
                    modifier = Modifier.padding(end = 8.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = updateBg),
                    contentPadding = PaddingValues(horizontal = 10.dp),
                    onClick = { onUpdateClick(item.module.updateInfo) },
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(4.dp),
                    ) {
                        Icon(
                            modifier = Modifier.size(20.dp),
                            imageVector = Icons.Default.CloudUpload,
                            tint = updateTint,
                            contentDescription = stringResource(CoreR.string.update),
                        )
                        Text(
                            text = stringResource(CoreR.string.update),
                            color = updateTint,
                            style = MaterialTheme.typography.bodyMedium,
                        )
                    }
                }
            }

            Button(
                colors = ButtonDefaults.buttonColors(containerColor = if (item.isRemoved) actionBg else removeBg),
                contentPadding = PaddingValues(horizontal = 10.dp),
                onClick = { viewModel.toggleRemove(item) },
                enabled = !item.isUpdated
            ) {
                val tint = if (item.isRemoved) actionIconTint else removeTint
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(4.dp),
                ) {
                    Icon(
                        modifier = Modifier.size(20.dp),
                        imageVector = if (item.isRemoved) Icons.AutoMirrored.Filled.Undo else Icons.Default.Delete,
                        tint = tint,
                        contentDescription = null
                    )
                    Text(
                        text = stringResource(
                            if (item.isRemoved) CoreR.string.module_state_restore
                            else CoreR.string.module_state_remove
                        ),
                        color = tint,
                        style = MaterialTheme.typography.bodyMedium,
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
    showDialog: MutableState<Boolean>,
    onDownload: (install: Boolean) -> Unit,
    onDismiss: () -> Unit,
) {
    val svc = ServiceLocator.networkService
    val title = stringResource(
        CoreR.string.repo_install_title,
        item.name, item.version, item.versionCode
    )

    if (showDialog.value) {
        AlertDialog(
            onDismissRequest = onDismiss,
            title = { Text(title) },
            text = {
                MarkdownTextAsync {
                    val str = svc.fetchString(item.changelog)
                    if (str.length > 1000) str.substring(0, 1000) else str
                }
            },
            confirmButton = {
                TextButton(
                    onClick = { onDownload(true) }
                ) {
                    Text(stringResource(CoreR.string.install))
                }
            },
            dismissButton = {
                Row {
                    TextButton(onClick = onDismiss) {
                        Text(stringResource(android.R.string.cancel))
                    }
                    Spacer(Modifier.weight(1f))
                    TextButton(onClick = { onDownload(false) }) {
                        Text(stringResource(CoreR.string.download))
                    }
                }
            }
        )
    }
}
