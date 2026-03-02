package com.topjohnwu.magisk.ui.compose.module

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Notification
import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts.OpenDocument
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
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
import timber.log.Timber
import java.io.IOException
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

    Box(modifier = Modifier.fillMaxSize()) {
        if (state.loading) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center), strokeCap = StrokeCap.Round)
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(bottom = 120.dp, start = 24.dp, end = 24.dp, top = 16.dp),
                verticalArrangement = Arrangement.spacedBy(24.dp)
            ) {
                // Actions Header
                item {
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                        FilledTonalButton(
                            onClick = { viewModel.refresh() },
                            modifier = Modifier.weight(1f).height(56.dp),
                            shape = RoundedCornerShape(topStart = 24.dp, bottomEnd = 24.dp, topEnd = 8.dp, bottomStart = 8.dp)
                        ) {
                            Icon(Icons.Rounded.Refresh, null, modifier = Modifier.size(20.dp))
                            Spacer(Modifier.width(8.dp))
                            Text("Refresh", fontWeight = FontWeight.Bold)
                        }
                        Button(
                            onClick = { zipPicker.launch(arrayOf("application/zip")) },
                            modifier = Modifier.weight(1f).height(56.dp),
                            shape = RoundedCornerShape(topEnd = 24.dp, bottomStart = 24.dp, topStart = 8.dp, bottomEnd = 8.dp)
                        ) {
                            Icon(Icons.Rounded.Add, null, modifier = Modifier.size(20.dp))
                            Spacer(Modifier.width(8.dp))
                            Text("Install", fontWeight = FontWeight.Bold)
                        }
                    }
                }

                if (state.modules.isEmpty()) {
                    item { EmptyStateView() }
                } else {
                    items(state.modules, key = { it.id }) { module ->
                        OrganicModuleCard(
                            module = module,
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
        
        SnackbarHost(hostState = snackbarHostState, modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 110.dp))
    }
}

@Composable
private fun OrganicModuleCard(
    module: ModuleUiItem,
    onToggleEnabled: () -> Unit,
    onToggleRemove: () -> Unit,
    onUpdate: () -> Unit,
    onAction: () -> Unit
) {
    val isEnabled = module.enabled && !module.removed
    
    Surface(
        modifier = Modifier.fillMaxWidth().animateContentSize(),
        shape = RoundedCornerShape(topStart = 48.dp, bottomEnd = 48.dp, topEnd = 12.dp, bottomStart = 12.dp),
        color = when {
            module.removed -> MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.2f)
            !module.enabled -> MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.6f)
            else -> MaterialTheme.colorScheme.surfaceContainerLow
        }
    ) {
        Column(modifier = Modifier.padding(28.dp).alpha(if (isEnabled) 1f else 0.8f)) {
            Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(module.name, style = MaterialTheme.typography.titleLarge, fontWeight = FontWeight.ExtraBold, maxLines = 2, overflow = TextOverflow.Ellipsis)
                    Spacer(Modifier.height(4.dp))
                    Surface(color = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f), shape = RoundedCornerShape(8.dp)) {
                        Text(text = module.versionAuthor, modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp), style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.primary, fontWeight = FontWeight.Bold)
                    }
                }
                Switch(checked = isEnabled, onCheckedChange = { onToggleEnabled() }, thumbContent = { if (isEnabled) Icon(Icons.Rounded.Check, null, Modifier.size(SwitchDefaults.IconSize)) })
            }

            if (module.badges.isNotEmpty()) {
                Spacer(Modifier.height(16.dp))
                Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    module.badges.forEach { badge ->
                        Surface(color = MaterialTheme.colorScheme.secondaryContainer, shape = RoundedCornerShape(12.dp)) {
                            Text(text = badge, modifier = Modifier.padding(horizontal = 10.dp, vertical = 6.dp), style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Black, color = MaterialTheme.colorScheme.onSecondaryContainer)
                        }
                    }
                }
            }
            
            if (module.description.isNotBlank()) {
                Spacer(Modifier.height(16.dp))
                Text(module.description, style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant, lineHeight = 22.sp, maxLines = 4, overflow = TextOverflow.Ellipsis)
            }

            if (module.noticeText != null) {
                Spacer(Modifier.height(20.dp))
                Surface(color = MaterialTheme.colorScheme.errorContainer, shape = RoundedCornerShape(20.dp), modifier = Modifier.fillMaxWidth()) {
                    Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
                        Icon(Icons.Rounded.Warning, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.onErrorContainer)
                        Spacer(Modifier.width(12.dp))
                        Text(module.noticeText, color = MaterialTheme.colorScheme.onErrorContainer, style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.SemiBold)
                    }
                }
            }

            Spacer(Modifier.height(28.dp))
            
            // Action Area: Smart layout to avoid compression
            Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                if (module.showAction) {
                    FilledTonalIconButton(onClick = onAction, modifier = Modifier.size(48.dp), shape = RoundedCornerShape(16.dp)) {
                        Icon(Icons.Rounded.Settings, null, modifier = Modifier.size(22.dp))
                    }
                    Spacer(Modifier.width(12.dp))
                }
                
                if (module.showUpdate) {
                    Button(
                        onClick = onUpdate,
                        enabled = module.updateReady,
                        shape = RoundedCornerShape(16.dp),
                        contentPadding = PaddingValues(horizontal = 16.dp),
                        modifier = Modifier.height(48.dp)
                    ) {
                        Icon(Icons.Rounded.SystemUpdateAlt, null, modifier = Modifier.size(18.dp))
                        Spacer(Modifier.width(8.dp))
                        Text("Update", fontWeight = FontWeight.Bold)
                    }
                }
                
                Spacer(Modifier.weight(1f))

                TextButton(
                    onClick = onToggleRemove, 
                    shape = RoundedCornerShape(16.dp),
                    colors = ButtonDefaults.textButtonColors(contentColor = if (module.removed) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.error)
                ) {
                    Icon(if (module.removed) Icons.Rounded.Refresh else Icons.Rounded.Delete, null, modifier = Modifier.size(20.dp))
                    Spacer(Modifier.width(8.dp))
                    Text(if (module.removed) "Restore" else "Remove", fontWeight = FontWeight.Bold)
                }
            }
        }
    }
}

@Composable
private fun EmptyStateView() {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .height(300.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Icon(Icons.Rounded.ExtensionOff, null, modifier = Modifier.size(80.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))
            Spacer(Modifier.height(16.dp))
            Text("No modules installed", style = MaterialTheme.typography.titleMedium, color = MaterialTheme.colorScheme.outline.copy(alpha = 0.5f))
        }
    }
}

// Logic components remain consistent
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
        if (update == null) { _state.update { it.copy(message = "Loading...") }; return }
        if (Info.isConnected.value != true) { _state.update { it.copy(message = "No connection") }; return }
        viewModelScope.launch(Dispatchers.IO) {
            runCatching {
                val file = MediaStoreUtils.getFile(update.downloadFilename)
                val response = networkService.fetchFile(update.zipUrl)
                response.byteStream().use { input ->
                    DownloadProcessor(object : DownloadNotifier { override val context = AppContext; override fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit) = Unit }).handleModule(input, file.uri)
                }
                file.uri
            }.onSuccess { uri -> withContext(Dispatchers.Main) { _state.update { it.copy(installUri = uri, message = "Download complete") } } }
            .onFailure { error -> withContext(Dispatchers.Main) { _state.update { it.copy(message = "Download failed") } } }
        }
    }

    fun postExternalRwDenied() { _state.update { it.copy(message = "Permission denied") } }
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
        zygiskUnloaded -> "Zygisk module not loaded"
        Info.isZygiskEnabled && isRiru -> "Suspended: $zygiskLabel active"
        !Info.isZygiskEnabled && isZygisk -> "Suspended: $zygiskLabel inactive"
        else -> null
    }
    return ModuleUiItem(id, name.ifBlank { id }, "$version by $author", description, enable, remove, updated, hasAction && noticeText == null, noticeText, updateInfo != null, outdated && !remove && enable, updateInfo, buildList { if (outdated) add("UPDATE"); if (updated) add("UPDATED"); if (remove) add("REMOVING"); if (!enable) add("DISABLED") })
}
