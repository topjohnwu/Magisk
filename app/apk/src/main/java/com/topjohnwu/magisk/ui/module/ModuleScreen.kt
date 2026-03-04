package com.topjohnwu.magisk.ui.module

import android.net.Uri
import android.os.SystemClock
import android.provider.OpenableColumns
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.NavigateNext
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.RefreshOnResume
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withTimeoutOrNull
import kotlinx.coroutines.withContext
import java.util.Locale
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun ModuleScreen(
    onInstallZip: (Uri) -> Unit = {},
    onRunAction: (String, String) -> Unit = { _, _ -> },
    viewModel: ModuleComposeViewModel = viewModel(factory = ModuleComposeViewModel.Factory)
) {
    val context = LocalContext.current
    val activity = context as? MainActivity
    val scope = rememberCoroutineScope()
    val state by viewModel.state.collectAsState()
    val contentEnterState = remember {
        MutableTransitionState(false).apply { targetState = true }
    }
    var query by rememberSaveable { mutableStateOf("") }
    var showSearch by rememberSaveable { mutableStateOf(false) }
    val snackbarHostState = remember { SnackbarHostState() }
    val localInstallDialog = rememberConfirmDialog()
    val confirmInstallTitle = stringResource(CoreR.string.confirm_install_title)
    var pendingOnlineModule by remember { mutableStateOf<OnlineModule?>(null) }
    val showOnlineDialog = rememberSaveable { mutableStateOf(false) }

    val zipPicker = rememberLauncherForActivityResult(ActivityResultContracts.GetContent()) { uri ->
        if (uri != null) {
            val displayName = context.contentResolver.query(uri, null, null, null, null)?.use { cursor ->
                val idx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                if (cursor.moveToFirst() && idx >= 0) cursor.getString(idx) else null
            } ?: uri.lastPathSegment ?: "module.zip"
            scope.launch {
                val result = localInstallDialog.awaitConfirm(
                    title = confirmInstallTitle,
                    content = context.getString(CoreR.string.confirm_install, displayName),
                )
                if (result == ConfirmResult.Confirmed) {
                    onInstallZip(uri)
                }
            }
        }
    }

    LaunchedEffect(Unit) { viewModel.refresh(force = true) }
    RefreshOnResume { viewModel.refresh(force = true) }
    
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }

    if (showOnlineDialog.value && pendingOnlineModule != null) {
        OnlineModuleDialog(
            item = pendingOnlineModule!!,
            showDialog = showOnlineDialog,
            onDownload = { install ->
                showOnlineDialog.value = false
                val host = activity
                if (host == null) {
                    viewModel.postMessageRes(CoreR.string.app_not_found)
                } else {
                    DownloadEngine.startWithActivity(
                        host, host.extension,
                        OnlineModuleSubject(pendingOnlineModule!!, install)
                    )
                }
                pendingOnlineModule = null
            },
            onDismiss = {
                showOnlineDialog.value = false
                pendingOnlineModule = null
            }
        )
    }

    val filteredModules = remember(state.modules, query) {
        val q = query.trim().lowercase(Locale.ROOT)
        if (q.isEmpty()) state.modules
        else state.modules.filter { it.searchKey.contains(q) }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        if (state.loading && state.modules.isEmpty()) {
            Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                CircularProgressIndicator(strokeCap = StrokeCap.Round)
            }
        } else {
            AnimatedVisibility(
                visibleState = contentEnterState,
                enter = fadeIn(animationSpec = tween(durationMillis = 220)) +
                    slideInVertically(
                        initialOffsetY = { it / 8 },
                        animationSpec = tween(durationMillis = 320, easing = FastOutSlowInEasing)
                    ),
                exit = fadeOut(animationSpec = tween(durationMillis = 120)),
                label = "moduleContentEnter"
            ) {
                LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    contentPadding = PaddingValues(bottom = 140.dp, top = 16.dp, start = 20.dp, end = 20.dp),
                    verticalArrangement = Arrangement.spacedBy(24.dp)
                ) {
                item {
                    Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                            Surface(
                                onClick = { showSearch = !showSearch },
                                modifier = Modifier.height(56.dp).weight(0.3f),
                                shape = RoundedCornerShape(16.dp),
                                color = MaterialTheme.colorScheme.secondaryContainer.copy(alpha = 0.7f)
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    Icon(if (showSearch) Icons.Rounded.Close else Icons.Rounded.Search, null, tint = MaterialTheme.colorScheme.onSecondaryContainer)
                                }
                            }
                            
                            Button(
                                onClick = { zipPicker.launch("application/zip") },
                                modifier = Modifier.weight(1f).height(56.dp),
                                shape = RoundedCornerShape(16.dp)
                            ) {
                                Icon(Icons.Rounded.FileUpload, null, modifier = Modifier.size(20.dp))
                                Spacer(Modifier.width(12.dp))
                                Text(
                                    text = stringResource(id = CoreR.string.module_action_install_external),
                                    fontWeight = FontWeight.Black
                                )
                            }
                        }
                        
                        AnimatedVisibility(
                            visible = showSearch,
                            enter = expandVertically() + fadeIn(),
                            exit = shrinkVertically() + fadeOut()
                        ) {
                            OutlinedTextField(
                                value = query,
                                onValueChange = { query = it },
                                modifier = Modifier.fillMaxWidth(),
                                shape = RoundedCornerShape(16.dp),
                                colors = OutlinedTextFieldDefaults.colors(
                                    focusedBorderColor = MaterialTheme.colorScheme.primary,
                                    unfocusedBorderColor = MaterialTheme.colorScheme.outlineVariant
                                ),
                                leadingIcon = { Icon(Icons.Rounded.Search, null) },
                                placeholder = { Text(stringResource(id = CoreR.string.modules_search_placeholder)) },
                                singleLine = true
                            )
                        }
                    }
                }

                if (state.modules.isEmpty()) {
                    item { 
                        Box(Modifier.fillParentMaxHeight(0.7f), contentAlignment = Alignment.Center) {
                            EmptyStateView() 
                        }
                    }
                } else if (filteredModules.isEmpty()) {
                    item {
                        Box(Modifier.fillMaxWidth().padding(32.dp), contentAlignment = Alignment.Center) {
                            Text(stringResource(id = CoreR.string.modules_no_results), style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f))
                        }
                    }
                } else {
                    itemsIndexed(filteredModules, key = { _, m -> m.id }) { _, module ->
                        StylishMagiskModuleCard(
                            module = module,
                            onToggleExpanded = { viewModel.toggleExpanded(module.id) },
                            onToggleEnabled = { viewModel.toggleEnabled(module.id) },
                            onToggleRemove = { viewModel.toggleRemove(module.id) },
                            onUpdate = { onlineModule ->
                                if (onlineModule == null) return@StylishMagiskModuleCard
                                if (Info.isConnected.value != true) {
                                    viewModel.postMessageRes(CoreR.string.no_connection)
                                    return@StylishMagiskModuleCard
                                }
                                pendingOnlineModule = onlineModule
                                showOnlineDialog.value = true
                            },
                            onAction = { onRunAction(module.id, module.name) }
                        )
                    }
                }
                }
            }
        }
        
        SnackbarHost(hostState = snackbarHostState, modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 120.dp))
    }
}

@Composable
private fun StylishMagiskModuleCard(
    module: ModuleUiItem,
    onToggleExpanded: () -> Unit,
    onToggleEnabled: () -> Unit,
    onToggleRemove: () -> Unit,
    onUpdate: (OnlineModule?) -> Unit,
    onAction: () -> Unit
) {
    val isEnabled = module.enabled && !module.removed
    val transition = updateTransition(targetState = module.expanded, label = "cardTransition")
    
    val elevation by transition.animateDp(
        transitionSpec = {
            spring(
                dampingRatio = Spring.DampingRatioNoBouncy,
                stiffness = Spring.StiffnessMediumLow
            )
        },
        label = "elevation"
    ) { if (it) 10.dp else 2.dp }
    val rotation by transition.animateFloat(
        transitionSpec = {
            spring(
                dampingRatio = Spring.DampingRatioLowBouncy,
                stiffness = Spring.StiffnessMediumLow
            )
        },
        label = "rotation"
    ) { if (it) 90f else 0f }
    val cardScale by transition.animateFloat(
        transitionSpec = {
            spring(
                dampingRatio = Spring.DampingRatioNoBouncy,
                stiffness = Spring.StiffnessMediumLow
            )
        },
        label = "cardScale"
    ) { if (it) 1f else 0.992f }
    val containerColor by animateColorAsState(
        targetValue = when {
            module.removed -> MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.5f)
            module.updated -> MaterialTheme.colorScheme.tertiaryContainer.copy(alpha = 0.65f)
            module.updateReady -> MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.8f)
            !isEnabled -> MaterialTheme.colorScheme.surfaceContainerLow
            else -> if (module.expanded) MaterialTheme.colorScheme.surfaceContainerHighest else MaterialTheme.colorScheme.surfaceContainerHigh
        },
        animationSpec = tween(durationMillis = 280, easing = FastOutSlowInEasing),
        label = "color"
    )
    val stateDotColor = if (module.updateReady) {
        MaterialTheme.colorScheme.tertiary
    } else {
        MaterialTheme.colorScheme.primary
    }

    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .animateContentSize(
                animationSpec = spring(
                    dampingRatio = Spring.DampingRatioNoBouncy,
                    stiffness = Spring.StiffnessMediumLow
                )
            )
            .scale(cardScale),
        shape = RoundedCornerShape(topEnd = 48.dp, bottomStart = 48.dp, topStart = 16.dp, bottomEnd = 16.dp),
        onClick = onToggleExpanded,
        colors = CardDefaults.elevatedCardColors(containerColor = containerColor),
        elevation = CardDefaults.elevatedCardElevation(defaultElevation = elevation)
    ) {
        Box {
            Icon(
                painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                contentDescription = null,
                modifier = Modifier.size(140.dp).align(Alignment.TopEnd).offset(x = 40.dp, y = (-30).dp).alpha(0.04f),
                tint = MaterialTheme.colorScheme.primary
            )
            if (module.removed || module.updated) {
                Icon(
                    imageVector = if (module.removed) Icons.Rounded.DeleteForever else Icons.Rounded.SystemUpdateAlt,
                    contentDescription = null,
                    modifier = Modifier
                        .size(128.dp)
                        .align(Alignment.CenterEnd)
                        .offset(x = 24.dp)
                        .alpha(0.08f),
                    tint = MaterialTheme.colorScheme.onSurface
                )
            }

            Column(modifier = Modifier.padding(24.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
                    Box(contentAlignment = Alignment.BottomEnd) {
                        Surface(
                            modifier = Modifier.size(56.dp),
                            shape = RoundedCornerShape(16.dp),
                            color = MaterialTheme.colorScheme.surface,
                            tonalElevation = 4.dp
                        ) {
                            Icon(Icons.Rounded.Extension, null, modifier = Modifier.padding(14.dp), tint = MaterialTheme.colorScheme.primary)
                        }
                        if (isEnabled) {
                            Box(
                                modifier = Modifier
                                    .size(18.dp)
                                    .clip(CircleShape)
                                    .background(stateDotColor)
                                    .border(2.dp, MaterialTheme.colorScheme.surface, CircleShape)
                            )
                        }
                    }
                    
                    Column(modifier = Modifier.weight(1f).padding(horizontal = 16.dp)) {
                        Text(
                            text = module.name,
                            style = MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Black,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis,
                            textDecoration = if (module.removed) TextDecoration.LineThrough else null,
                            color = if (isEnabled) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f)
                        )
                        Spacer(Modifier.height(4.dp))
                        Surface(
                            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.12f),
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text(
                                text = module.versionAuthor,
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.primary,
                                fontWeight = FontWeight.Black,
                                textDecoration = if (module.removed) TextDecoration.LineThrough else null,
                                modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp).alpha(if (isEnabled) 1f else 0.7f)
                            )
                        }
                        if (module.badges.isNotEmpty()) {
                            Spacer(Modifier.height(8.dp))
                            Text(
                                text = module.badges.joinToString(" | "),
                                style = MaterialTheme.typography.labelSmall,
                                fontWeight = FontWeight.SemiBold,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                    }
                    
                    AnimatedVisibility(
                        visible = module.expanded,
                        enter = fadeIn(animationSpec = tween(durationMillis = 180, delayMillis = 50)) +
                            scaleIn(
                                initialScale = 0.85f,
                                animationSpec = spring(
                                    dampingRatio = Spring.DampingRatioNoBouncy,
                                    stiffness = Spring.StiffnessMediumLow
                                )
                            ),
                        exit = fadeOut(animationSpec = tween(durationMillis = 120)) +
                            scaleOut(targetScale = 0.85f, animationSpec = tween(durationMillis = 120))
                    ) {
                        Switch(
                            checked = isEnabled,
                            onCheckedChange = { onToggleEnabled() },
                            thumbContent = if (isEnabled) {
                                { Icon(Icons.Rounded.Check, null, Modifier.size(16.dp)) }
                            } else null
                        )
                    }
                    
                    Icon(
                        imageVector = Icons.AutoMirrored.Rounded.NavigateNext,
                        contentDescription = null,
                        modifier = Modifier.rotate(rotation).padding(start = 12.dp),
                        tint = MaterialTheme.colorScheme.outline
                    )
                }

                AnimatedVisibility(
                    visible = module.expanded,
                    enter = expandVertically(
                        expandFrom = Alignment.Top,
                        animationSpec = spring(
                            dampingRatio = Spring.DampingRatioNoBouncy,
                            stiffness = Spring.StiffnessLow
                        )
                    ) + slideInVertically(
                        initialOffsetY = { it / 10 },
                        animationSpec = tween(durationMillis = 260, easing = FastOutSlowInEasing)
                    ) + fadeIn(animationSpec = tween(durationMillis = 200, delayMillis = 40)),
                    exit = shrinkVertically(
                        shrinkTowards = Alignment.Top,
                        animationSpec = tween(durationMillis = 210, easing = FastOutLinearInEasing)
                    ) + fadeOut(animationSpec = tween(durationMillis = 120)),
                    label = "moduleCardDetails"
                ) {
                    Column {
                        if (module.description.isNotBlank()) {
                            Spacer(Modifier.height(20.dp))
                            Text(
                                text = module.description,
                                style = MaterialTheme.typography.bodyLarge,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                lineHeight = 24.sp,
                                textDecoration = if (module.removed) TextDecoration.LineThrough else null,
                                modifier = Modifier.alpha(if (isEnabled) 1f else 0.8f)
                            )
                        }

                        if (module.noticeText != null) {
                            Spacer(Modifier.height(20.dp))
                            Surface(
                                color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.6f),
                                shape = RoundedCornerShape(16.dp),
                                modifier = Modifier.fillMaxWidth()
                            ) {
                                Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
                                    Icon(Icons.Rounded.Warning, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.error)
                                    Spacer(Modifier.width(12.dp))
                                    Text(module.noticeText, color = MaterialTheme.colorScheme.onErrorContainer, style = MaterialTheme.typography.bodyMedium, fontWeight = FontWeight.Bold)
                                }
                            }
                        }

                        Spacer(Modifier.height(32.dp))
                        
                        // ALL BUTTONS ALWAYS AT RIGHT
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(12.dp, Alignment.End)
                        ) {
                            if (module.showAction) {
                                FilledTonalIconButton(
                                    onClick = onAction,
                                    modifier = Modifier
                                        .size(52.dp)
                                        .animateEnterExit(
                                            enter = slideInHorizontally(
                                                initialOffsetX = { fullWidth -> fullWidth / 3 },
                                                animationSpec = tween(durationMillis = 220, delayMillis = 60, easing = FastOutSlowInEasing)
                                            ) + fadeIn(animationSpec = tween(durationMillis = 180, delayMillis = 60)),
                                            exit = slideOutHorizontally(
                                                targetOffsetX = { fullWidth -> fullWidth / 3 },
                                                animationSpec = tween(durationMillis = 120, easing = FastOutLinearInEasing)
                                            ) + fadeOut(animationSpec = tween(durationMillis = 100))
                                        ),
                                    shape = RoundedCornerShape(16.dp)
                                ) { Icon(Icons.Rounded.Settings, null, modifier = Modifier.size(24.dp)) }
                            }

                            if (module.showUpdate) {
                                Button(
                                    onClick = { onUpdate(module.update) },
                                    enabled = module.updateReady,
                                    shape = RoundedCornerShape(16.dp),
                                    modifier = Modifier
                                        .height(52.dp)
                                        .animateEnterExit(
                                            enter = slideInHorizontally(
                                                initialOffsetX = { fullWidth -> fullWidth / 3 },
                                                animationSpec = tween(durationMillis = 220, delayMillis = 110, easing = FastOutSlowInEasing)
                                            ) + fadeIn(animationSpec = tween(durationMillis = 180, delayMillis = 110)),
                                            exit = slideOutHorizontally(
                                                targetOffsetX = { fullWidth -> fullWidth / 3 },
                                                animationSpec = tween(durationMillis = 120, easing = FastOutLinearInEasing)
                                            ) + fadeOut(animationSpec = tween(durationMillis = 100))
                                        )
                                ) {
                                    Icon(Icons.Rounded.SystemUpdateAlt, null, modifier = Modifier.size(20.dp))
                                    Spacer(Modifier.width(8.dp))
                                    Text(stringResource(id = CoreR.string.update), fontWeight = FontWeight.Black)
                                }
                            }

                            Surface(
                                onClick = onToggleRemove,
                                enabled = !module.updated,
                                modifier = Modifier
                                    .size(52.dp)
                                    .alpha(if (module.updated) 0.45f else 1f)
                                    .animateEnterExit(
                                        enter = slideInHorizontally(
                                            initialOffsetX = { fullWidth -> fullWidth / 3 },
                                            animationSpec = tween(durationMillis = 220, delayMillis = 150, easing = FastOutSlowInEasing)
                                        ) + fadeIn(animationSpec = tween(durationMillis = 180, delayMillis = 150)),
                                        exit = slideOutHorizontally(
                                            targetOffsetX = { fullWidth -> fullWidth / 3 },
                                            animationSpec = tween(durationMillis = 120, easing = FastOutLinearInEasing)
                                        ) + fadeOut(animationSpec = tween(durationMillis = 100))
                                    ),
                                shape = CircleShape,
                                color = when {
                                    module.updated -> MaterialTheme.colorScheme.surfaceVariant
                                    module.removed -> MaterialTheme.colorScheme.primaryContainer
                                    else -> MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.2f)
                                }
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    Icon(
                                        imageVector = if (module.removed) Icons.Rounded.SettingsBackupRestore else Icons.Rounded.DeleteForever,
                                        contentDescription = null,
                                        tint = when {
                                            module.updated -> MaterialTheme.colorScheme.onSurfaceVariant
                                            module.removed -> MaterialTheme.colorScheme.primary
                                            else -> MaterialTheme.colorScheme.error
                                        },
                                        modifier = Modifier.size(24.dp)
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun EmptyStateView() {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Surface(
            modifier = Modifier.size(140.dp),
            shape = RoundedCornerShape(48.dp),
            color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(Icons.Rounded.ExtensionOff, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.4f))
            }
        }
        Spacer(Modifier.height(32.dp))
        Text(stringResource(id = CoreR.string.module_empty), style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Black, color = MaterialTheme.colorScheme.outline)
        Spacer(Modifier.height(8.dp))
        Text(stringResource(id = CoreR.string.module_action_install_external), style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.outline.copy(alpha = 0.6f))
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
    val changelog by produceState(initialValue = AppContext.getString(CoreR.string.loading), item.changelog) {
        val text = runCatching {
            withContext(Dispatchers.IO) { svc.fetchString(item.changelog) }
        }.getOrDefault("")
        value = if (text.length > 1000) text.substring(0, 1000) else text
    }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title, style = MaterialTheme.typography.titleLarge, fontWeight = FontWeight.Black) },
        text = {
            Text(
                text = changelog,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        },
        confirmButton = {
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                TextButton(onClick = { onDownload(false) }) {
                    Text(stringResource(CoreR.string.download))
                }
                Button(onClick = { onDownload(true) }) {
                    Text(stringResource(CoreR.string.install))
                }
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

data class ModuleUiItem(
    val id: String,
    val name: String,
    val versionAuthor: String,
    val description: String,
    val enabled: Boolean,
    val removed: Boolean,
    val updated: Boolean,
    val showAction: Boolean,
    val noticeText: String?,
    val showUpdate: Boolean,
    val updateReady: Boolean,
    val update: OnlineModule?,
    val badges: List<String>,
    val searchKey: String,
    val expanded: Boolean = false
)
data class ModuleUiState(val loading: Boolean = true, val modules: List<ModuleUiItem> = emptyList())

class ModuleComposeViewModel(private val moduleProvider: suspend () -> List<LocalModule>) : ViewModel() {
    private val _state = MutableStateFlow(ModuleUiState())
    val state: StateFlow<ModuleUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    private var refreshJob: Job? = null
    private var metadataJob: Job? = null
    private val moduleCache = linkedMapOf<String, LocalModule>()
    private val cacheLock = Any()
    private var lastRefreshAt = 0L
    private var lastMetadataRefreshAt = 0L

    fun refresh(force: Boolean = false) {
        val now = SystemClock.elapsedRealtime()
        if (!force && _state.value.modules.isNotEmpty() && now - lastRefreshAt < MIN_REFRESH_INTERVAL_MS) {
            return
        }
        lastRefreshAt = now
        refreshJob?.cancel()
        metadataJob?.cancel()
        val hadModules = _state.value.modules.isNotEmpty()
        refreshJob = viewModelScope.launch {
            if (!hadModules) {
                _state.update { it.copy(loading = true) }
            }
            val list = if (Info.env.isActive && isModuleRepoLoaded()) readInstalledModules() else emptyList()
            synchronized(cacheLock) {
                moduleCache.clear()
                list.forEach { moduleCache[it.id] = it }
            }
            val currentExpanded = _state.value.modules.filter { it.expanded }.map { it.id }.toSet()
            _state.update { it.copy(loading = false, modules = list.map { it.toUiItem(currentExpanded.contains(it.id)) }) }
            if (list.isNotEmpty() && now - lastMetadataRefreshAt >= MIN_METADATA_REFRESH_INTERVAL_MS) {
                lastMetadataRefreshAt = now
                metadataJob = launch(Dispatchers.IO) {
                    list.forEach { runCatching { it.fetch() } }
                    val currentExpandedMetadata = _state.value.modules.filter { it.expanded }.map { it.id }.toSet()
                    val updatedUi = list.map { it.toUiItem(currentExpandedMetadata.contains(it.id)) }
                    withContext(Dispatchers.Main) {
                        _state.update { st -> st.copy(modules = updatedUi) }
                    }
                }
            }
        }
    }

    fun toggleExpanded(id: String) {
        _state.update { st ->
            st.copy(modules = st.modules.map { 
                if (it.id == id) it.copy(expanded = !it.expanded) else it 
            })
        }
    }

    fun toggleEnabled(id: String) = updateModule(id) { it.enable = !it.enable }
    fun toggleRemove(id: String) = updateModule(id) { it.remove = !it.remove }
    fun postMessageRes(@androidx.annotation.StringRes res: Int) {
        _messages.tryEmit(AppContext.getString(res))
    }

    private fun updateModule(id: String, block: (LocalModule) -> Unit) {
        viewModelScope.launch(Dispatchers.IO) {
            val module = synchronized(cacheLock) { moduleCache[id] } ?: run {
                val list = readInstalledModules()
                synchronized(cacheLock) {
                    moduleCache.clear()
                    list.forEach { moduleCache[it.id] = it }
                }
                val currentExp = _state.value.modules.filter { it.expanded }.map { it.id }.toSet()
                withContext(Dispatchers.Main) {
                    _state.update { it.copy(modules = list.map { m -> m.toUiItem(currentExp.contains(m.id)) }) }
                }
                synchronized(cacheLock) { moduleCache[id] } ?: return@launch
            }
            val ok = runCatching { block(module) }.isSuccess
            if (!ok) {
                withContext(Dispatchers.Main) {
                    _messages.emit(AppContext.getString(CoreR.string.failure))
                }
                return@launch
            }
            val currentExpanded = _state.value.modules.find { it.id == id }?.expanded ?: false
            val updatedUi = module.toUiItem(currentExpanded)
            withContext(Dispatchers.Main) {
                _state.update { st ->
                    val index = st.modules.indexOfFirst { it.id == id }
                    if (index < 0) st
                    else {
                        val copy = st.modules.toMutableList()
                        copy[index] = updatedUi
                        st.copy(modules = copy)
                    }
                }
            }
        }
    }

    private suspend fun isModuleRepoLoaded(): Boolean = withTimeoutOrNull(3000) { withContext(Dispatchers.IO) { LocalModule.loaded() } } ?: false
    private suspend fun readInstalledModules(): List<LocalModule> = withTimeoutOrNull(5000) { withContext(Dispatchers.IO) { moduleProvider() } } ?: emptyList()

    object Factory : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return ModuleComposeViewModel { LocalModule.installed() } as T } }

    companion object {
        private const val MIN_REFRESH_INTERVAL_MS = 1200L
        private const val MIN_METADATA_REFRESH_INTERVAL_MS = 30_000L
    }
}

private fun LocalModule.toUiItem(expanded: Boolean = false): ModuleUiItem {
    val zygiskLabel = AppContext.getString(CoreR.string.zygisk)
    val safeName = name.ifBlank { id }
    val safeDescription = description
    val noticeText: String? = when {
        zygiskUnloaded -> AppContext.getString(CoreR.string.zygisk_module_unloaded)
        Info.isZygiskEnabled && isRiru -> AppContext.getString(CoreR.string.suspend_text_riru, zygiskLabel)
        !Info.isZygiskEnabled && isZygisk -> AppContext.getString(CoreR.string.suspend_text_zygisk, zygiskLabel)
        else -> null
    }
    return ModuleUiItem(
        id,
        safeName,
        AppContext.getString(CoreR.string.module_version_author, version, author),
        safeDescription,
        enable,
        remove,
        updated,
        hasAction && noticeText == null,
        noticeText,
        updateInfo != null,
        outdated && !remove && enable,
        updateInfo,
        buildList {
            if (outdated) add(AppContext.getString(CoreR.string.module_badge_update))
            if (updated) add(AppContext.getString(CoreR.string.module_badge_updated))
            if (remove) add(AppContext.getString(CoreR.string.module_badge_removing))
            if (!enable) add(AppContext.getString(CoreR.string.module_badge_disabled))
        },
        buildString {
            append(safeName.lowercase(Locale.ROOT))
            append('\n')
            append(id.lowercase(Locale.ROOT))
            append('\n')
            append(safeDescription.lowercase(Locale.ROOT))
        },
        expanded
    )
}
