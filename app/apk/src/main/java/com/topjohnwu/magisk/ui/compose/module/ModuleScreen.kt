package com.topjohnwu.magisk.ui.compose.module

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Notification
import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts.OpenDocument
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadNotifier
import com.topjohnwu.magisk.core.download.DownloadProcessor
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
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
    val activity = LocalContext.current as? UIActivity<*>
    val lifecycleOwner = LocalLifecycleOwner.current
    val state by viewModel.state.collectAsState()
    var query by rememberSaveable { mutableStateOf("") }
    var showSearch by rememberSaveable { mutableStateOf(false) }
    val snackbarHostState = remember { SnackbarHostState() }
    val zipPicker = rememberLauncherForActivityResult(OpenDocument()) { uri ->
        if (uri != null) onInstallZip(uri)
    }

    LaunchedEffect(state.message) {
        state.message?.let {
            snackbarHostState.showSnackbar(it)
            viewModel.consumeMessage()
        }
    }
    LaunchedEffect(state.installUri) {
        state.installUri?.let { uri ->
            onInstallZip(uri)
            viewModel.consumeInstallUri()
        }
    }
    LaunchedEffect(Unit) { viewModel.refresh() }
    DisposableEffect(lifecycleOwner) {
        val observer = LifecycleEventObserver { _, event ->
            if (event == Lifecycle.Event.ON_RESUME) viewModel.refresh()
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
    }

    val filteredModules = remember(state.modules, query) {
        val q = query.trim().lowercase(Locale.getDefault())
        if (q.isEmpty()) state.modules
        else state.modules.filter { module ->
            module.name.lowercase(Locale.getDefault()).contains(q) ||
                module.id.lowercase(Locale.getDefault()).contains(q) ||
                module.description.lowercase(Locale.getDefault()).contains(q)
        }
    }

    val shapes = listOf(
        RoundedCornerShape(topStart = 48.dp, topEnd = 12.dp, bottomStart = 12.dp, bottomEnd = 48.dp),
        RoundedCornerShape(topStart = 12.dp, topEnd = 48.dp, bottomStart = 48.dp, bottomEnd = 12.dp),
        RoundedCornerShape(topStart = 32.dp, topEnd = 32.dp, bottomStart = 8.dp, bottomEnd = 32.dp),
        RoundedCornerShape(topStart = 12.dp, topEnd = 56.dp, bottomStart = 56.dp, bottomEnd = 56.dp)
    )

    Box(modifier = Modifier.fillMaxSize()) {
        if (state.loading) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center), strokeCap = StrokeCap.Round)
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(bottom = 140.dp, start = 20.dp, end = 20.dp, top = 16.dp),
                verticalArrangement = Arrangement.spacedBy(20.dp)
            ) {
                item {
                    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                            Surface(
                                onClick = { showSearch = !showSearch },
                                modifier = Modifier.height(64.dp).weight(0.4f),
                                shape = RoundedCornerShape(32.dp, 8.dp, 8.dp, 32.dp),
                                color = MaterialTheme.colorScheme.secondaryContainer
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    Icon(if (showSearch) Icons.Rounded.Close else Icons.Rounded.Search, null)
                                }
                            }
                            
                            Button(
                                onClick = { zipPicker.launch(arrayOf("application/zip")) },
                                modifier = Modifier.weight(1f).height(64.dp),
                                shape = RoundedCornerShape(8.dp, 32.dp, 32.dp, 8.dp)
                            ) {
                                Icon(Icons.Rounded.Add, null, modifier = Modifier.size(22.dp))
                                Spacer(Modifier.width(12.dp))
                                Text(stringResource(id = CoreR.string.install), fontWeight = FontWeight.Black, fontSize = 16.sp)
                            }
                        }
                        
                        AnimatedVisibility(
                            visible = showSearch,
                            enter = expandVertically() + fadeIn(),
                            exit = shrinkVertically() + fadeOut()
                        ) {
                            TextField(
                                value = query,
                                onValueChange = { query = it },
                                modifier = Modifier.fillMaxWidth(),
                                shape = RoundedCornerShape(24.dp),
                                colors = TextFieldDefaults.colors(
                                    focusedIndicatorColor = Color.Transparent,
                                    unfocusedIndicatorColor = Color.Transparent
                                ),
                                leadingIcon = { Icon(Icons.Rounded.Search, null) },
                                placeholder = { Text(stringResource(id = CoreR.string.modules_search_placeholder)) },
                                singleLine = true
                            )
                        }
                    }
                }

                if (state.modules.isEmpty()) {
                    item { EmptyStateView() }
                } else if (filteredModules.isEmpty()) {
                    item {
                        Box(Modifier.fillMaxWidth().padding(32.dp), contentAlignment = Alignment.Center) {
                            Text(stringResource(id = CoreR.string.modules_no_results), style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
                        }
                    }
                } else {
                    itemsIndexed(filteredModules, key = { _, m -> m.id }) { index, module ->
                        OrganicModuleCard(
                            module = module,
                            shape = shapes[index % shapes.size],
                            onToggleEnabled = { viewModel.toggleEnabled(module.id) },
                            onToggleRemove = { viewModel.toggleRemove(module.id) },
                            onUpdate = {
                                activity?.withPermission(WRITE_EXTERNAL_STORAGE) { if (it) viewModel.markUpdate(module.id) else viewModel.postExternalRwDenied() }
                                ?: viewModel.markUpdate(module.id)
                            },
                            onAction = { onRunAction(module.id, module.name) }
                        )
                    }
                }
            }
        }
        
        SnackbarHost(hostState = snackbarHostState, modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 120.dp))
    }
}

@Composable
private fun OrganicModuleCard(
    module: ModuleUiItem,
    shape: Shape,
    onToggleEnabled: () -> Unit,
    onToggleRemove: () -> Unit,
    onUpdate: () -> Unit,
    onAction: () -> Unit
) {
    val isEnabled = module.enabled && !module.removed
    
    Surface(
        modifier = Modifier.fillMaxWidth().animateContentSize(),
        shape = shape,
        color = when {
            module.removed -> MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.3f)
            module.updateReady -> MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f)
            !module.enabled -> MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f)
            else -> MaterialTheme.colorScheme.surfaceContainerHigh
        },
        tonalElevation = if (module.updateReady) 8.dp else 2.dp
    ) {
        Column(modifier = Modifier.padding(24.dp)) {
            Row(verticalAlignment = Alignment.Top, modifier = Modifier.fillMaxWidth()) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = module.name,
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Black,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis,
                        color = if (isEnabled) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f)
                    )
                    Spacer(Modifier.height(6.dp))
                    Text(
                        text = module.versionAuthor,
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.primary,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.alpha(if (isEnabled) 1f else 0.6f)
                    )
                }
                
                Switch(
                    checked = isEnabled,
                    onCheckedChange = { onToggleEnabled() },
                    colors = SwitchDefaults.colors(
                        checkedThumbColor = MaterialTheme.colorScheme.primary,
                        checkedTrackColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.2f)
                    )
                )
            }

            if (module.badges.isNotEmpty()) {
                Spacer(Modifier.height(16.dp))
                Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    module.badges.forEach { badge ->
                        Surface(
                            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f),
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text(
                                text = badge,
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                                style = MaterialTheme.typography.labelSmall,
                                fontWeight = FontWeight.ExtraBold,
                                color = MaterialTheme.colorScheme.primary
                            )
                        }
                    }
                }
            }
            
            if (module.description.isNotBlank()) {
                Spacer(Modifier.height(16.dp))
                Text(
                    text = module.description,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    lineHeight = 20.sp,
                    maxLines = 3,
                    overflow = TextOverflow.Ellipsis,
                    modifier = Modifier.alpha(if (isEnabled) 1f else 0.7f)
                )
            }

            if (module.noticeText != null) {
                Spacer(Modifier.height(16.dp))
                Surface(
                    color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.5f),
                    shape = RoundedCornerShape(12.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Row(modifier = Modifier.padding(12.dp), verticalAlignment = Alignment.CenterVertically) {
                        Icon(Icons.Rounded.Info, null, modifier = Modifier.size(18.dp), tint = MaterialTheme.colorScheme.error)
                        Spacer(Modifier.width(10.dp))
                        Text(module.noticeText, color = MaterialTheme.colorScheme.onErrorContainer, style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Bold)
                    }
                }
            }

            Spacer(Modifier.height(24.dp))
            
            Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                if (module.showAction) {
                    FilledTonalIconButton(
                        onClick = onAction,
                        modifier = Modifier.size(44.dp),
                        shape = RoundedCornerShape(12.dp)
                    ) {
                        Icon(Icons.Rounded.Settings, null, modifier = Modifier.size(20.dp))
                    }
                    Spacer(Modifier.width(12.dp))
                }
                
                if (module.showUpdate) {
                    Button(
                        onClick = onUpdate,
                        enabled = module.updateReady,
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier.height(44.dp),
                        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)
                    ) {
                        Icon(Icons.Rounded.SystemUpdateAlt, null, modifier = Modifier.size(18.dp))
                        Spacer(Modifier.width(8.dp))
                        Text(stringResource(id = CoreR.string.module_badge_update), fontWeight = FontWeight.Black)
                    }
                }
                
                Spacer(Modifier.weight(1f))

                IconButton(
                    onClick = onToggleRemove,
                    modifier = Modifier.background(
                        if (module.removed) MaterialTheme.colorScheme.primary.copy(alpha = 0.1f) 
                        else MaterialTheme.colorScheme.error.copy(alpha = 0.1f),
                        CircleShape
                    )
                ) {
                    Icon(
                        imageVector = if (module.removed) Icons.Rounded.SettingsBackupRestore else Icons.Rounded.DeleteForever,
                        contentDescription = null,
                        tint = if (module.removed) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.error,
                        modifier = Modifier.size(22.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun EmptyStateView() {
    Box(modifier = Modifier.fillMaxWidth().height(400.dp), contentAlignment = Alignment.Center) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Surface(
                modifier = Modifier.size(120.dp),
                shape = RoundedCornerShape(40.dp),
                color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(Icons.Rounded.ExtensionOff, null, modifier = Modifier.size(56.dp), tint = MaterialTheme.colorScheme.outline)
                }
            }
            Spacer(Modifier.height(24.dp))
            Text(stringResource(id = CoreR.string.module_empty), style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Bold, color = MaterialTheme.colorScheme.outline)
            Text(stringResource(id = CoreR.string.module_action_install_external), style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.outline.copy(alpha = 0.7f))
        }
    }
}

data class ModuleUiItem(val id: String, val name: String, val versionAuthor: String, val description: String, val enabled: Boolean, val removed: Boolean, val updated: Boolean, val showAction: Boolean, val noticeText: String?, val showUpdate: Boolean, val updateReady: Boolean, val update: OnlineModule?, val badges: List<String>)
data class ModuleUiState(val loading: Boolean = true, val modules: List<ModuleUiItem> = emptyList(), val installUri: Uri? = null, val message: String? = null)

class ModuleComposeViewModel(private val networkService: NetworkService, private val moduleProvider: suspend () -> List<LocalModule>) : ViewModel() {
    private val _state = MutableStateFlow(ModuleUiState())
    val state: StateFlow<ModuleUiState> = _state

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            val list = if (Info.env.isActive && isModuleRepoLoaded()) readInstalledModules() else emptyList()
            _state.update { it.copy(loading = false, modules = list.map { it.toUiItem() }) }
            if (list.isNotEmpty()) {
                launch(Dispatchers.IO) {
                    list.forEach { runCatching { it.fetch() } }
                    withContext(Dispatchers.Main) { _state.update { it.copy(modules = list.map { it.toUiItem() }) } }
                }
            }
        }
    }

    fun toggleEnabled(id: String) = updateModule(id) { it.enable = !it.enable }
    fun toggleRemove(id: String) = updateModule(id) { it.remove = !it.remove }
    fun markUpdate(id: String) {
        val update = _state.value.modules.firstOrNull { it.id == id }?.update
        if (update == null) { _state.update { it.copy(message = AppContext.getString(CoreR.string.loading)) }; return }
        if (Info.isConnected.value != true) { _state.update { it.copy(message = AppContext.getString(CoreR.string.no_connection)) }; return }
        viewModelScope.launch(Dispatchers.IO) {
            runCatching {
                val file = MediaStoreUtils.getFile(update.downloadFilename)
                val response = networkService.fetchFile(update.zipUrl)
                response.byteStream().use { input ->
                    DownloadProcessor(object : DownloadNotifier { override val context = AppContext; override fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit) = Unit }).handleModule(input, file.uri)
                }
                file.uri
            }.onSuccess { uri -> withContext(Dispatchers.Main) { _state.update { it.copy(installUri = uri, message = AppContext.getString(CoreR.string.download_complete)) } } }
            .onFailure { withContext(Dispatchers.Main) { _state.update { it.copy(message = AppContext.getString(CoreR.string.download_file_error)) } } }
        }
    }

    fun postExternalRwDenied() { _state.update { it.copy(message = AppContext.getString(CoreR.string.external_rw_permission_denied)) } }
    private fun updateModule(id: String, block: (LocalModule) -> Unit) { viewModelScope.launch { val modules = readInstalledModules().toMutableList().apply { find { it.id == id }?.let(block) }; _state.update { it.copy(modules = modules.map { it.toUiItem() }) } } }
    private suspend fun isModuleRepoLoaded(): Boolean = withTimeoutOrNull(3000) { withContext(Dispatchers.IO) { LocalModule.loaded() } } ?: false
    private suspend fun readInstalledModules(): List<LocalModule> = withTimeoutOrNull(5000) { withContext(Dispatchers.IO) { moduleProvider() } } ?: emptyList()
    fun consumeMessage() { _state.update { it.copy(message = null) } }
    fun consumeInstallUri() { _state.update { it.copy(installUri = null) } }

    object Factory : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return ModuleComposeViewModel(ServiceLocator.networkService) { LocalModule.installed() } as T } }
}

private fun LocalModule.toUiItem(): ModuleUiItem {
    val zygiskLabel = "Zygisk"
    val noticeText: String? = when {
        zygiskUnloaded -> AppContext.getString(CoreR.string.zygisk_module_unloaded)
        Info.isZygiskEnabled && isRiru -> AppContext.getString(CoreR.string.suspend_text_riru, zygiskLabel)
        !Info.isZygiskEnabled && isZygisk -> AppContext.getString(CoreR.string.suspend_text_zygisk, zygiskLabel)
        else -> null
    }
    return ModuleUiItem(
        id,
        name.ifBlank { id },
        AppContext.getString(CoreR.string.module_version_author, version, author),
        description,
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
        }
    )
}

