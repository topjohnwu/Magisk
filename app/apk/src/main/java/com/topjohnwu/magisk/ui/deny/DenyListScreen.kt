package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import androidx.annotation.StringRes
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.NavigateNext
import androidx.compose.material.icons.rounded.Check
import androidx.compose.material.icons.rounded.Search
import androidx.compose.material.icons.rounded.SettingsSuggest
import androidx.compose.material.icons.rounded.SwapVert
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.res.painterResource
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
import com.topjohnwu.magisk.ui.RefreshOnResume
import com.topjohnwu.magisk.ui.deny.AppProcessInfo
import com.topjohnwu.magisk.ui.deny.CmdlineListItem
import com.topjohnwu.superuser.Shell
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@Composable
fun DenyListScreen(
    onBack: () -> Unit,
    viewModel: DenyListComposeViewModel = viewModel(factory = DenyListComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }

    RefreshOnResume { viewModel.refresh() }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 20.dp),
            contentPadding = PaddingValues(top = 16.dp, bottom = 140.dp),
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            item {
                DenyListSearchSection(
                    query = state.query,
                    onQueryChange = viewModel::setQuery,
                    showSystem = state.showSystem,
                    onToggleSystem = { viewModel.setShowSystem(!state.showSystem) },
                    showOs = state.showOs,
                    onToggleOs = { viewModel.setShowOs(!state.showOs) },
                    sortMethod = state.sortMethod,
                    onSortMethodSelected = viewModel::setSortMethod
                )
            }
            when {
                state.loading && state.items.isEmpty() -> {
                    item {
                        Box(
                            modifier = Modifier
                                .fillParentMaxWidth()
                                .fillParentMaxHeight(0.7f),
                            contentAlignment = Alignment.Center
                        ) {
                            CircularProgressIndicator(strokeCap = StrokeCap.Round)
                        }
                    }
                }
                state.items.isEmpty() -> {
                    item {
                        EmptyDenyListState()
                    }
                }
                else -> {
                    items(state.items, key = { it.packageName }) { item ->
                        StylishDenyListCard(
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

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 110.dp)
        )
    }
}

@Composable
private fun DenyListSearchSection(
    query: String,
    onQueryChange: (String) -> Unit,
    showSystem: Boolean,
    onToggleSystem: () -> Unit,
    showOs: Boolean,
    onToggleOs: () -> Unit,
    sortMethod: DenyListSortMethod,
    onSortMethodSelected: (DenyListSortMethod) -> Unit
) {
    var showSortMenu by remember { mutableStateOf(false) }
    val chipColors = FilterChipDefaults.filterChipColors(
        containerColor = MaterialTheme.colorScheme.surfaceContainerHighest,
        labelColor = MaterialTheme.colorScheme.onSurfaceVariant,
        selectedContainerColor = MaterialTheme.colorScheme.primary,
        selectedLabelColor = MaterialTheme.colorScheme.onPrimary
    )
    Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(id = CoreR.string.denylist_search_filters),
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Black,
                letterSpacing = 1.2.sp,
                color = MaterialTheme.colorScheme.outline,
                modifier = Modifier.weight(1f)
            )

            Box {
                AssistChip(
                    onClick = { showSortMenu = true },
                    label = {
                        Text(
                            text = stringResource(id = sortMethod.labelRes),
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    },
                    leadingIcon = {
                        Icon(
                            imageVector = Icons.Rounded.SwapVert,
                            contentDescription = null,
                            modifier = Modifier.size(16.dp)
                        )
                    },
                    shape = RoundedCornerShape(12.dp)
                )

                DropdownMenu(
                    expanded = showSortMenu,
                    onDismissRequest = { showSortMenu = false }
                ) {
                    DenyListSortMethod.entries.forEach { method ->
                        DropdownMenuItem(
                            text = { Text(stringResource(id = method.labelRes)) },
                            onClick = {
                                onSortMethodSelected(method)
                                showSortMenu = false
                            },
                            leadingIcon = if (method == sortMethod) {
                                { Icon(Icons.Rounded.Check, contentDescription = null) }
                            } else {
                                null
                            }
                        )
                    }
                }
            }
        }

        OutlinedTextField(
            value = query,
            onValueChange = onQueryChange,
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            placeholder = { Text(text = AppContext.getString(CoreR.string.hide_filter_hint)) },
            leadingIcon = { Icon(Icons.Rounded.Search, null) },
            shape = RoundedCornerShape(16.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = MaterialTheme.colorScheme.primary,
                unfocusedBorderColor = MaterialTheme.colorScheme.outlineVariant
            )
        )
        
        Row(
            modifier = Modifier.fillMaxWidth().horizontalScroll(rememberScrollState()),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            FilterChip(
                selected = showSystem,
                onClick = onToggleSystem,
                label = { Text(stringResource(id = CoreR.string.show_system_app), fontWeight = FontWeight.Black) },
                shape = RoundedCornerShape(12.dp),
                colors = chipColors
            )
            FilterChip(
                selected = showOs,
                enabled = showSystem,
                onClick = onToggleOs,
                label = { Text(stringResource(id = CoreR.string.show_os_app), fontWeight = FontWeight.Black) },
                shape = RoundedCornerShape(12.dp),
                colors = chipColors
            )
        }
    }
}

@Composable
private fun StylishDenyListCard(
    item: DenyListAppUi,
    onToggleExpanded: () -> Unit,
    onToggleApp: () -> Unit,
    onToggleProcess: (DenyListProcessUi) -> Unit
) {
    val isAnyChecked = item.checkedCount > 0
    val transition = updateTransition(targetState = item.expanded, label = "cardTransition")
    
    val elevation by transition.animateDp(label = "elevation") { if (it) 10.dp else 2.dp }
    val rotation by transition.animateFloat(label = "rotation") { if (it) 90f else 0f }
    val cardScale by transition.animateFloat(label = "cardScale") { if (it) 1f else 0.992f }
    
    val containerColor by animateColorAsState(
        targetValue = when {
            item.expanded -> MaterialTheme.colorScheme.surfaceContainerHighest
            isAnyChecked -> MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f)
            else -> MaterialTheme.colorScheme.surfaceContainerHigh
        },
        animationSpec = tween(durationMillis = 280, easing = FastOutSlowInEasing),
        label = "color"
    )

    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .animateContentSize(spring(stiffness = Spring.StiffnessMediumLow))
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

            Column(modifier = Modifier.padding(24.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
                    Box(contentAlignment = Alignment.BottomEnd) {
                        Surface(
                            modifier = Modifier.size(56.dp),
                            shape = RoundedCornerShape(16.dp),
                            color = MaterialTheme.colorScheme.surface,
                            tonalElevation = 4.dp
                        ) {
                            Image(
                                painter = BitmapPainter(item.icon.toBitmap().asImageBitmap()),
                                contentDescription = null,
                                modifier = Modifier.padding(8.dp)
                            )
                        }
                        if (isAnyChecked) {
                            Box(
                                modifier = Modifier
                                    .size(18.dp)
                                    .clip(CircleShape)
                                    .background(MaterialTheme.colorScheme.primary)
                                    .border(2.dp, MaterialTheme.colorScheme.surface, CircleShape)
                            )
                        }
                    }
                    
                    Column(modifier = Modifier.weight(1f).padding(horizontal = 16.dp)) {
                        Text(
                            text = item.label,
                            style = MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Black,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        Spacer(Modifier.height(4.dp))
                        Surface(
                            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.12f),
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text(
                                text = item.packageName,
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.primary,
                                fontWeight = FontWeight.Black,
                                modifier = Modifier.padding(horizontal = 10.dp, vertical = 4.dp),
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                        
                        if (isAnyChecked) {
                            Spacer(Modifier.height(8.dp))
                            Text(
                                text = "${item.checkedCount}/${item.processes.size} ATTIVI",
                                style = MaterialTheme.typography.labelSmall,
                                fontWeight = FontWeight.Bold,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                letterSpacing = 0.5.sp
                            )
                        }
                    }
                    
                    AnimatedVisibility(
                        visible = item.expanded,
                        enter = fadeIn() + scaleIn(initialScale = 0.85f),
                        exit = fadeOut() + scaleOut(targetScale = 0.85f)
                    ) {
                        TriStateCheckbox(
                            state = item.selectionState, 
                            onClick = onToggleApp,
                            colors = CheckboxDefaults.colors(checkedColor = MaterialTheme.colorScheme.primary)
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
                    visible = item.expanded,
                    enter = expandVertically(expandFrom = Alignment.Top) + slideInVertically(initialOffsetY = { it / 10 }) + fadeIn(),
                    exit = shrinkVertically(shrinkTowards = Alignment.Top) + fadeOut()
                ) {
                    Column {
                        Spacer(Modifier.height(24.dp))
                        item.processes.forEach { process ->
                            Surface(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .padding(vertical = 4.dp)
                                    .clip(RoundedCornerShape(16.dp))
                                    .clickable { onToggleProcess(process) },
                                color = if (process.enabled) MaterialTheme.colorScheme.primary.copy(alpha = 0.08f) 
                                        else Color.Transparent,
                                shape = RoundedCornerShape(16.dp)
                            ) {
                                Row(
                                    modifier = Modifier.padding(12.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Checkbox(
                                        checked = process.enabled, 
                                        onCheckedChange = { onToggleProcess(process) }
                                    )
                                    Spacer(modifier = Modifier.width(12.dp))
                                    Column {
                                        Text(
                                            text = process.displayName, 
                                            style = MaterialTheme.typography.bodyLarge, 
                                            fontWeight = if (process.enabled) FontWeight.Black else FontWeight.Medium,
                                            color = if (process.enabled) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurface
                                        )
                                        if (process.packageName != item.packageName) {
                                            Text(
                                                text = process.packageName, 
                                                style = MaterialTheme.typography.labelSmall, 
                                                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f)
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
    }
}

@Composable
private fun EmptyDenyListState() {
    Column(horizontalAlignment = Alignment.CenterHorizontally, modifier = Modifier.fillMaxWidth().padding(top = 64.dp)) {
        Surface(
            modifier = Modifier.size(140.dp),
            shape = RoundedCornerShape(48.dp),
            color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(Icons.Rounded.SettingsSuggest, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.4f))
            }
        }
        Spacer(Modifier.height(32.dp))
        Text(AppContext.getString(CoreR.string.log_data_none), style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Black, color = MaterialTheme.colorScheme.outline)
    }
}

// Logic components remain identical
data class DenyListProcessUi(val name: String, val packageName: String, val isIsolated: Boolean, val isAppZygote: Boolean, val defaultSelection: Boolean, val enabled: Boolean) {
    val displayName: String get() = if (isIsolated) AppContext.getString(CoreR.string.isolated_process_label, name) else name
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

enum class DenyListSortMethod(@StringRes val labelRes: Int) {
    ActiveFirst(CoreR.string.denylist_sort_active_first),
    NameAsc(CoreR.string.denylist_sort_name_asc),
    NameDesc(CoreR.string.denylist_sort_name_desc)
}

data class DenyListUiState(
    val loading: Boolean = true,
    val query: String = "",
    val showSystem: Boolean = false,
    val showOs: Boolean = false,
    val sortMethod: DenyListSortMethod = DenyListSortMethod.ActiveFirst,
    val items: List<DenyListAppUi> = emptyList()
)

class DenyListComposeViewModel : ViewModel() {
    private val _state = MutableStateFlow(DenyListUiState())
    val state: StateFlow<DenyListUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    private var allApps: List<DenyListAppUi> = emptyList()
    private var refreshJob: Job? = null

    fun refresh() {
        refreshJob?.cancel()
        refreshJob = viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            runCatching { loadApps() }.onSuccess { loaded ->
                allApps = loaded
                _state.update { it.copy(loading = false) }
                applyFilters()
            }.onFailure {
                _state.update { it.copy(loading = false) }
                _messages.tryEmit(it.message ?: AppContext.getString(CoreR.string.failure))
            }
        }
    }

    fun setQuery(v: String) { _state.update { it.copy(query = v) }; applyFilters() }
    fun setShowSystem(v: Boolean) { _state.update { it.copy(showSystem = v, showOs = if (v) it.showOs else false) }; applyFilters() }
    fun setShowOs(v: Boolean) { _state.update { it.copy(showOs = v) }; applyFilters() }
    fun setSortMethod(v: DenyListSortMethod) { _state.update { it.copy(sortMethod = v) }; applyFilters() }
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
            var success = true
            val affected = if (enabled) {
                if (app.expanded) app.processes else app.processes.filter { it.defaultSelection }
            } else {
                app.processes
            }

            if (enabled) {
                affected.filter { !it.enabled }.forEach { p ->
                    success = Shell.cmd("magisk --denylist add ${p.packageName} ${shellQuote(p.name)}")
                        .exec().isSuccess && success
                }
            } else {
                // Match legacy behavior: clear package-level denylist first.
                success = Shell.cmd("magisk --denylist rm $pkg").exec().isSuccess
                if (success) {
                    // Keep explicit cleanup for isolated processes.
                    affected.filter { it.enabled && it.isIsolated }.forEach { p ->
                        success = Shell.cmd("magisk --denylist rm ${p.packageName} ${shellQuote(p.name)}")
                            .exec().isSuccess && success
                    }
                }
            }

            withContext(Dispatchers.Main) {
                if (success) {
                    allApps = allApps.map { a ->
                        if (a.packageName != pkg) a else a.copy(
                            processes = a.processes.map { p ->
                                if (affected.any { t -> t.name == p.name && t.packageName == p.packageName }) {
                                    p.copy(enabled = enabled)
                                } else {
                                    p
                                }
                            }
                        )
                    }
                    applyFilters()
                } else { postFailure() }
            }
        }
    }

    private fun postFailure() { _messages.tryEmit(AppContext.getString(CoreR.string.failure)) }

    private fun applyFilters() {
        val st = _state.value
        val filtered = allApps.asSequence().filter { app ->
            // Keep expanded cards visible while interacting, so they do not disappear/jump mid-toggle.
            val passesVisibility = app.expanded || app.checkedCount > 0 || (st.showSystem || !app.isSystem) && (app.isAppUid || st.showSystem && st.showOs)
            passesVisibility && (st.query.isBlank() || app.label.contains(st.query, true) || app.packageName.contains(st.query, true) || app.processes.any { it.name.contains(st.query, true) })
        }.toList()

        val sorted = when (st.sortMethod) {
            DenyListSortMethod.ActiveFirst -> filtered.sortedWith(compareBy({ it.checkedCount == 0 }, { it.label.lowercase() }, { it.packageName }))
            DenyListSortMethod.NameAsc -> filtered.sortedWith(compareBy({ it.label.lowercase() }, { it.packageName }))
            DenyListSortMethod.NameDesc -> filtered.sortedWith(compareByDescending<DenyListAppUi> { it.label.lowercase() }.thenByDescending { it.packageName })
        }

        _state.update { it.copy(items = sorted) }
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
