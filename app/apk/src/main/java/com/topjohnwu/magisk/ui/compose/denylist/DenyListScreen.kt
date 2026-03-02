package com.topjohnwu.magisk.ui.compose.denylist

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import androidx.activity.compose.BackHandler
import androidx.compose.animation.*
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ArrowRight
import androidx.compose.material.icons.rounded.Search
import androidx.compose.material.icons.rounded.SettingsSuggest
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.state.ToggleableState
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.graphics.drawable.toBitmap
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.ui.deny.AppProcessInfo
import com.topjohnwu.magisk.ui.deny.CmdlineListItem
import com.topjohnwu.superuser.Shell
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.Locale

@Composable
fun DenyListScreen(
    onBack: () -> Unit,
    viewModel: DenyListComposeViewModel = viewModel(factory = DenyListComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }

    BackHandler(onBack = onBack)

    LaunchedEffect(Unit) { viewModel.refresh() }
    LaunchedEffect(state.message) {
        state.message?.let {
            snackbarHostState.showSnackbar(it)
            viewModel.consumeMessage()
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 24.dp, vertical = 12.dp)
        ) {
            OutlinedTextField(
                value = state.query,
                onValueChange = viewModel::setQuery,
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                placeholder = { Text(text = AppContext.getString(CoreR.string.hide_search)) },
                shape = RoundedCornerShape(20.dp),
                leadingIcon = { Icon(Icons.Rounded.Search, null) },
                colors = OutlinedTextFieldDefaults.colors(
                    unfocusedBorderColor = MaterialTheme.colorScheme.outlineVariant,
                    focusedBorderColor = MaterialTheme.colorScheme.primary
                )
            )
            
            Spacer(modifier = Modifier.height(16.dp))

            Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                FilterChip(
                    selected = state.showSystem,
                    onClick = { viewModel.setShowSystem(!state.showSystem) },
                    label = { Text(stringResource(id = CoreR.string.show_system_app), fontWeight = FontWeight.Bold) },
                    shape = RoundedCornerShape(12.dp)
                )
                FilterChip(
                    selected = state.showOs,
                    enabled = state.showSystem,
                    onClick = { viewModel.setShowOs(!state.showOs) },
                    label = { Text(stringResource(id = CoreR.string.show_os_app), fontWeight = FontWeight.Bold) },
                    shape = RoundedCornerShape(12.dp)
                )
            }

            Spacer(modifier = Modifier.height(20.dp))

            when {
                state.loading -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        CircularProgressIndicator(strokeCap = androidx.compose.ui.graphics.StrokeCap.Round)
                    }
                }
                state.items.isEmpty() -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Icon(Icons.Rounded.SettingsSuggest, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f))
                            Spacer(Modifier.height(16.dp))
                            Text(AppContext.getString(CoreR.string.log_data_none), color = MaterialTheme.colorScheme.outline)
                        }
                    }
                }
                else -> {
                    LazyColumn(
                        modifier = Modifier.fillMaxSize(),
                        contentPadding = PaddingValues(bottom = 32.dp),
                        verticalArrangement = Arrangement.spacedBy(16.dp)
                    ) {
                        items(state.items, key = { it.packageName }) { item ->
                            OrganicDenyListCard(
                                item = item,
                                onToggleExpanded = { viewModel.toggleExpanded(item.packageName) },
                                onToggleApp = {
                                    viewModel.setAppChecked(
                                        item.packageName,
                                        item.selectionState != ToggleableState.On
                                    )
                                },
                                onToggleProcess = { process ->
                                    viewModel.toggleProcess(
                                        item.packageName,
                                        process.name,
                                        process.packageName
                                    )
                                }
                            )
                        }
                    }
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 24.dp)
        )
    }
}

@Composable
private fun OrganicDenyListCard(
    item: DenyListAppUi,
    onToggleExpanded: () -> Unit,
    onToggleApp: () -> Unit,
    onToggleProcess: (DenyListProcessUi) -> Unit
) {
    val isAnyChecked = item.checkedCount > 0
    val rotation by animateFloatAsState(if (item.expanded) 90f else 0f, label = "rotation")

    ElevatedCard(
        modifier = Modifier.fillMaxWidth().animateContentSize(),
        shape = RoundedCornerShape(topEnd = 40.dp, bottomStart = 40.dp, topStart = 12.dp, bottomEnd = 12.dp),
        colors = CardDefaults.elevatedCardColors(
            containerColor = if (isAnyChecked) MaterialTheme.colorScheme.surfaceContainerHigh else MaterialTheme.colorScheme.surfaceContainerLow
        )
    ) {
        Column(modifier = Modifier.padding(20.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Image(
                    painter = BitmapPainter(item.icon.toBitmap().asImageBitmap()),
                    contentDescription = null,
                    modifier = Modifier.size(48.dp).clip(RoundedCornerShape(12.dp)).background(MaterialTheme.colorScheme.surface)
                )
                Spacer(modifier = Modifier.width(16.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(item.label, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.ExtraBold, maxLines = 1, overflow = TextOverflow.Ellipsis)
                    Text(item.packageName, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.primary, fontWeight = FontWeight.Bold, maxLines = 1)
                    
                    Surface(
                        color = MaterialTheme.colorScheme.secondaryContainer.copy(alpha = 0.5f),
                        shape = RoundedCornerShape(8.dp),
                        modifier = Modifier.padding(top = 4.dp)
                    ) {
                        Text(
                            text = stringResource(
                                id = CoreR.string.denylist_process_count,
                                item.checkedCount,
                                item.processes.size
                            ),
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp),
                            style = MaterialTheme.typography.labelSmall,
                            fontWeight = FontWeight.Black
                        )
                    }
                }
                
                TriStateCheckbox(state = item.selectionState, onClick = onToggleApp)
                
                IconButton(onClick = onToggleExpanded) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Rounded.ArrowRight,
                        contentDescription = null,
                        modifier = Modifier.rotate(rotation),
                        tint = MaterialTheme.colorScheme.outline
                    )
                }
            }

            AnimatedVisibility(
                visible = item.expanded,
                enter = expandVertically() + fadeIn(),
                exit = shrinkVertically() + fadeOut()
            ) {
                Column {
                    HorizontalDivider(Modifier.padding(vertical = 16.dp), color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f))
                    item.processes.forEach { process ->
                        Row(
                            modifier = Modifier.fillMaxWidth().clickable { onToggleProcess(process) }.padding(vertical = 8.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Checkbox(checked = process.enabled, onCheckedChange = { onToggleProcess(process) })
                            Spacer(modifier = Modifier.width(12.dp))
                            Column {
                                Text(process.displayName, style = MaterialTheme.typography.bodyMedium, fontWeight = FontWeight.Bold)
                                Text(process.packageName, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
                            }
                        }
                    }
                }
            }
        }
    }
}

// Data structures and ViewModel remain unchanged for compatibility
data class DenyListProcessUi(val name: String, val packageName: String, val isIsolated: Boolean, val isAppZygote: Boolean, val defaultSelection: Boolean, val enabled: Boolean) {
    val displayName: String get() = if (isIsolated) "(isolated) $name" else name
}

data class DenyListAppUi(val packageName: String, val label: String, val icon: Drawable, val isSystem: Boolean, val isAppUid: Boolean, val expanded: Boolean, val processes: List<DenyListProcessUi>) {
    val checkedCount: Int get() = processes.count { it.enabled }
    val selectionState: ToggleableState get() {
        val targets = if (expanded) processes else processes.filter { it.defaultSelection }
        if (targets.isEmpty()) return ToggleableState.Off
        val enabled = targets.count { it.enabled }
        return when {
            enabled == 0 -> ToggleableState.Off
            enabled == targets.size -> ToggleableState.On
            else -> ToggleableState.Indeterminate
        }
    }
}

data class DenyListUiState(val loading: Boolean = true, val query: String = "", val showSystem: Boolean = false, val showOs: Boolean = false, val items: List<DenyListAppUi> = emptyList(), val message: String? = null)

class DenyListComposeViewModel : ViewModel() {
    private val _state = MutableStateFlow(DenyListUiState())
    val state: StateFlow<DenyListUiState> = _state
    private var allApps: List<DenyListAppUi> = emptyList()

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            runCatching { loadApps() }.onSuccess { loaded ->
                allApps = loaded
                _state.update { it.copy(loading = false) }
                applyFilters()
            }.onFailure { _state.update { it.copy(loading = false, message = it.message ?: AppContext.getString(CoreR.string.failure)) } }
        }
    }

    fun setQuery(v: String) { _state.update { it.copy(query = v) }; applyFilters() }
    fun setShowSystem(v: Boolean) { _state.update { it.copy(showSystem = v, showOs = if (v) it.showOs else false) }; applyFilters() }
    fun setShowOs(v: Boolean) { _state.update { it.copy(showOs = v) }; applyFilters() }
    fun toggleExpanded(pkg: String) { allApps = allApps.map { if (it.packageName == pkg) it.copy(expanded = !it.expanded) else it }; applyFilters() }

    fun toggleProcess(pkg: String, name: String, pPkg: String) {
        viewModelScope.launch(Dispatchers.IO) {
            val app = allApps.firstOrNull { it.packageName == pkg } ?: return@launch
            val process = app.processes.firstOrNull { it.name == name && it.packageName == pPkg } ?: return@launch
            val enabled = !process.enabled
            val cmd = if (enabled) "add" else "rm"
            val result = Shell.cmd("magisk --denylist $cmd $pPkg ${shellQuote(name)}").exec()
            withContext(Dispatchers.Main) {
                if (result.isSuccess) {
                    allApps = allApps.map { a -> if (a.packageName != pkg) a else a.copy(processes = a.processes.map { p -> if (p.name == name && p.packageName == pPkg) p.copy(enabled = enabled) else p }) }
                    applyFilters()
                } else { postFailure() }
            }
        }
    }

    fun setAppChecked(pkg: String, enabled: Boolean) {
        viewModelScope.launch(Dispatchers.IO) {
            val app = allApps.firstOrNull { it.packageName == pkg } ?: return@launch
            val targets = if (app.expanded) app.processes else app.processes.filter { it.defaultSelection }
            var success = true
            targets.filter { it.enabled != enabled }.forEach { p ->
                val cmd = if (enabled) "add" else "rm"
                success = Shell.cmd("magisk --denylist $cmd ${p.packageName} ${shellQuote(p.name)}").exec().isSuccess && success
            }
            withContext(Dispatchers.Main) {
                if (success) {
                    allApps = allApps.map { a -> if (a.packageName != pkg) a else a.copy(processes = a.processes.map { p -> if (targets.any { t -> t.name == p.name }) p.copy(enabled = enabled) else p }) }
                    applyFilters()
                } else { postFailure() }
            }
        }
    }

    fun consumeMessage() { _state.update { it.copy(message = null) } }
    private fun postFailure() { _state.update { it.copy(message = AppContext.getString(CoreR.string.failure)) } }

    private fun applyFilters() {
        val st = _state.value
        val filtered = allApps.asSequence().filter { app ->
            val passesVisibility = app.checkedCount > 0 || (st.showSystem || !app.isSystem) && (app.isAppUid || st.showSystem && st.showOs)
            passesVisibility && (st.query.isBlank() || app.label.contains(st.query, true) || app.packageName.contains(st.query, true) || app.processes.any { it.name.contains(st.query, true) })
        }.sortedWith(compareBy({ it.checkedCount == 0 }, { it.label.lowercase() }, { it.packageName })).toList()
        _state.update { it.copy(items = filtered) }
    }

    @SuppressLint("InlinedApi")
    private suspend fun loadApps(): List<DenyListAppUi> = withContext(Dispatchers.Default) {
        val pm = AppContext.packageManager
        val denyList = Shell.cmd("magisk --denylist ls").exec().out.map { CmdlineListItem(it) }
        pm.getInstalledApplications(PackageManager.MATCH_UNINSTALLED_PACKAGES).asSequence()
            .filter { it.packageName != AppContext.packageName }
            .mapNotNull { app -> runCatching {
                val proc = AppProcessInfo(app, pm, denyList)
                if (proc.processes.isEmpty()) null else DenyListAppUi(app.packageName, proc.label, proc.iconImage, proc.isSystemApp(), proc.isApp(), false, proc.processes.map { DenyListProcessUi(it.name, it.packageName, it.isIsolated, it.isAppZygote, it.isIsolated || it.isAppZygote || it.name == it.packageName, it.isEnabled) })
            }.getOrNull() }.toList()
    }

    companion object {
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return DenyListComposeViewModel() as T
            }
        }
    }
}

private fun shellQuote(v: String): String = "'${v.replace("'", "'\\''")}'"
