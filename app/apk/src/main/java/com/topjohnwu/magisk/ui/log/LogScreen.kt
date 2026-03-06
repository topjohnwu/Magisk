package com.topjohnwu.magisk.ui.log

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.spring
import androidx.compose.animation.expandVertically
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.scaleIn
import androidx.compose.animation.scaleOut
import androidx.compose.animation.shrinkVertically
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.BugReport
import androidx.compose.material.icons.rounded.Close
import androidx.compose.material.icons.rounded.Dangerous
import androidx.compose.material.icons.rounded.DeleteSweep
import androidx.compose.material.icons.rounded.History
import androidx.compose.material.icons.rounded.Info
import androidx.compose.material.icons.rounded.KeyboardDoubleArrowDown
import androidx.compose.material.icons.rounded.Refresh
import androidx.compose.material.icons.rounded.SaveAlt
import androidx.compose.material.icons.rounded.Search
import androidx.compose.material.icons.rounded.Warning
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilterChipDefaults
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.FloatingActionButtonDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.IconButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ui.RefreshOnResume
import com.topjohnwu.magisk.ui.terminal.ansiLogText
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun LogsScreen(
    viewModel: MagiskLogViewModel = viewModel(factory = MagiskLogViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    val activity = LocalContext.current as? UIActivity<*>
    val listState = rememberLazyListState()
    val scope = rememberCoroutineScope()

    var filter by remember { mutableStateOf(LogDisplayFilter.ALL) }
    var searchQuery by remember { mutableStateOf("") }
    var showSearch by remember { mutableStateOf(false) }

    val filteredLogs = remember(state.visibleLogs, filter, searchQuery) {
        val base = when (filter) {
            LogDisplayFilter.ALL -> state.visibleLogs
            LogDisplayFilter.ISSUES -> state.visibleLogs.filter { it.isIssue }
            LogDisplayFilter.MAGISK -> state.visibleLogs.filter { it.isMagisk }
            LogDisplayFilter.SU -> state.visibleLogs.filter { it.isSu }
        }
        if (searchQuery.isEmpty()) base
        else base.filter { it.contains(searchQuery) }
    }

    val canScrollDown by remember { derivedStateOf { listState.canScrollForward } }

    RefreshOnResume { viewModel.refresh() }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }

    val shapes = listOf(
        RoundedCornerShape(topStart = 48.dp, topEnd = 12.dp, bottomStart = 12.dp, bottomEnd = 48.dp),
        RoundedCornerShape(topStart = 12.dp, topEnd = 48.dp, bottomStart = 48.dp, bottomEnd = 12.dp),
        RoundedCornerShape(topStart = 32.dp, topEnd = 32.dp, bottomStart = 8.dp, bottomEnd = 32.dp),
        RoundedCornerShape(topStart = 12.dp, topEnd = 56.dp, bottomStart = 56.dp, bottomEnd = 56.dp)
    )

    Box(modifier = Modifier.fillMaxSize().navigationBarsPadding()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 20.dp)
                .padding(top = 12.dp)
        ) {
            // Action Row
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                Surface(
                    onClick = { showSearch = !showSearch },
                    modifier = Modifier.height(64.dp).weight(0.35f),
                    shape = RoundedCornerShape(32.dp, 8.dp, 8.dp, 32.dp),
                    color = MaterialTheme.colorScheme.secondaryContainer,
                    tonalElevation = 2.dp
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        AnimatedContent(
                            targetState = showSearch,
                            transitionSpec = {
                                (fadeIn() + scaleIn()).togetherWith(fadeOut() + scaleOut())
                            },
                            label = "searchIcon"
                        ) { isSearch ->
                            Icon(
                                imageVector = if (isSearch) Icons.Rounded.Close else Icons.Rounded.Search, 
                                contentDescription = null,
                                tint = MaterialTheme.colorScheme.onSecondaryContainer
                            )
                        }
                    }
                }
                
                Surface(
                    modifier = Modifier.weight(1f).height(64.dp),
                    shape = RoundedCornerShape(8.dp, 32.dp, 32.dp, 8.dp),
                    color = MaterialTheme.colorScheme.primary,
                    tonalElevation = 4.dp
                ) {
                    Row(
                        modifier = Modifier.fillMaxSize().padding(horizontal = 8.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceEvenly
                    ) {
                        IconButton(
                            onClick = viewModel::refresh,
                            modifier = Modifier.size(40.dp),
                            colors = IconButtonDefaults.iconButtonColors(contentColor = MaterialTheme.colorScheme.onPrimary)
                        ) { Icon(Icons.Rounded.Refresh, null, modifier = Modifier.size(22.dp)) }
                        
                        IconButton(
                            onClick = { activity?.withPermission(WRITE_EXTERNAL_STORAGE) { if (it) viewModel.saveMagiskLog() } },
                            modifier = Modifier.size(40.dp),
                            colors = IconButtonDefaults.iconButtonColors(contentColor = MaterialTheme.colorScheme.onPrimary)
                        ) { Icon(Icons.Rounded.SaveAlt, null, modifier = Modifier.size(22.dp)) }
                        
                        IconButton(
                            onClick = viewModel::clearMagiskLogs, 
                            modifier = Modifier.size(40.dp),
                            colors = IconButtonDefaults.iconButtonColors(contentColor = MaterialTheme.colorScheme.onPrimary)
                        ) { Icon(Icons.Rounded.DeleteSweep, null, modifier = Modifier.size(22.dp)) }
                    }
                }
            }

            AnimatedVisibility(
                visible = showSearch,
                enter = expandVertically(animationSpec = spring(stiffness = Spring.StiffnessLow)) + fadeIn(),
                exit = shrinkVertically(animationSpec = spring(stiffness = Spring.StiffnessLow)) + fadeOut()
            ) {
                Column {
                    Spacer(Modifier.height(12.dp))
                    TextField(
                        value = searchQuery,
                        onValueChange = { searchQuery = it },
                        modifier = Modifier.fillMaxWidth(),
                        placeholder = { Text("Search logs...") },
                        leadingIcon = { Icon(Icons.Rounded.Search, null) },
                        trailingIcon = { 
                            if (searchQuery.isNotEmpty()) {
                                IconButton(onClick = { searchQuery = "" }) { Icon(Icons.Rounded.Close, null) }
                            } 
                        },
                        shape = RoundedCornerShape(24.dp),
                        colors = TextFieldDefaults.colors(
                            focusedIndicatorColor = Color.Transparent, 
                            unfocusedIndicatorColor = Color.Transparent,
                            focusedContainerColor = MaterialTheme.colorScheme.surfaceContainerHigh,
                            unfocusedContainerColor = MaterialTheme.colorScheme.surfaceContainerHigh
                        ),
                        singleLine = true
                    )
                }
            }

            Spacer(Modifier.height(16.dp))

            // Filter Chips - Now Centered
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(rememberScrollState()),
                horizontalArrangement = Arrangement.spacedBy(8.dp, Alignment.CenterHorizontally)
            ) {
                LogFilterChip(selected = filter == LogDisplayFilter.ALL, label = "Tutti", badge = state.visibleLogs.size, onClick = { filter = LogDisplayFilter.ALL })
                LogFilterChip(selected = filter == LogDisplayFilter.ISSUES, label = "Problemi", badge = state.visibleLogs.count { it.isIssue }, onClick = { filter = LogDisplayFilter.ISSUES }, isError = true)
                LogFilterChip(selected = filter == LogDisplayFilter.MAGISK, label = "Magisk", badge = state.visibleLogs.count { it.isMagisk }, onClick = { filter = LogDisplayFilter.MAGISK })
                LogFilterChip(selected = filter == LogDisplayFilter.SU, label = "su", badge = state.visibleLogs.count { it.isSu }, onClick = { filter = LogDisplayFilter.SU })
            }

            Spacer(Modifier.height(16.dp))

            // Log Content
            if (state.loading && state.visibleLogs.isEmpty()) {
                Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator(strokeCap = StrokeCap.Round)
                }
            } else if (filteredLogs.isEmpty()) {
                EmptyLogState("Nessun log trovato")
            } else {
                LazyColumn(
                    state = listState,
                    modifier = Modifier.fillMaxWidth().weight(1f),
                    contentPadding = PaddingValues(bottom = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    itemsIndexed(filteredLogs, key = { _, item -> item.id }) { index, item ->
                        LogEntryOrganicCard(item, shapes[index % shapes.size])
                    }
                }
            }
        }

        // FAB to Scroll to Bottom
        AnimatedVisibility(
            visible = canScrollDown, 
            modifier = Modifier.align(Alignment.BottomEnd).padding(24.dp),
            enter = scaleIn(animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy)) + fadeIn(),
            exit = scaleOut() + fadeOut()
        ) {
            FloatingActionButton(
                onClick = { scope.launch { if (filteredLogs.isNotEmpty()) listState.animateScrollToItem(filteredLogs.lastIndex) } },
                shape = CircleShape,
                containerColor = MaterialTheme.colorScheme.primaryContainer,
                elevation = FloatingActionButtonDefaults.elevation(defaultElevation = 6.dp)
            ) { Icon(Icons.Rounded.KeyboardDoubleArrowDown, null) }
        }

        SnackbarHost(hostState = snackbarHostState, modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 16.dp))
    }
}

@Composable
private fun LogFilterChip(selected: Boolean, label: String, badge: Int, onClick: () -> Unit, isError: Boolean = false) {
    FilterChip(
        selected = selected,
        onClick = onClick,
        label = { 
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(label)
                if (badge > 0) {
                    Spacer(Modifier.width(6.dp))
                    Text(badge.toString(), style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Black, modifier = Modifier.alpha(0.6f))
                }
            }
        },
        shape = RoundedCornerShape(12.dp),
        colors = if (isError && !selected) FilterChipDefaults.filterChipColors(labelColor = MaterialTheme.colorScheme.error) else FilterChipDefaults.filterChipColors()
    )
}

@Composable
private fun LogEntryOrganicCard(item: MagiskLogUiItem, shape: Shape) {
    val levelColor = item.level.color()
    val isMagisk = item.isMagisk
    
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = shape,
        color = if (isMagisk) MaterialTheme.colorScheme.surfaceContainerHighest else MaterialTheme.colorScheme.surfaceContainerHigh,
        tonalElevation = if (isMagisk) 8.dp else 1.dp,
        border = if (isMagisk) BorderStroke(1.dp, levelColor.copy(alpha = 0.3f)) else null
    ) {
        Row(modifier = Modifier.height(IntrinsicSize.Min)) {
            if (item.isIssue) {
                Box(modifier = Modifier.width(6.dp).fillMaxHeight().background(levelColor))
            }
            Column(modifier = Modifier.padding(16.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Surface(
                        color = levelColor.copy(alpha = 0.15f),
                        shape = CircleShape,
                        modifier = Modifier.size(32.dp)
                    ) {
                        Box(contentAlignment = Alignment.Center) {
                            Icon(item.level.icon(), null, tint = levelColor, modifier = Modifier.size(18.dp))
                        }
                    }
                    Spacer(Modifier.width(12.dp))
                    Column(modifier = Modifier.weight(1f)) {
                        Text(
                            text = item.tag,
                            style = MaterialTheme.typography.labelLarge,
                            fontWeight = FontWeight.Black,
                            color = if (isMagisk) MaterialTheme.colorScheme.primary else levelColor,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        if (item.timestamp.isNotBlank()) {
                            Text(item.timestamp, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.outline)
                        }
                    }
                    
                    if (item.pid != 0) {
                        Surface(
                            color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f), 
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text("P:${item.pid}", modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp), style = MaterialTheme.typography.labelSmall, fontSize = 10.sp, fontFamily = FontFamily.Monospace)
                        }
                    }
                }
                
                Spacer(Modifier.height(12.dp))
                
                Surface(
                    color = MaterialTheme.colorScheme.surfaceContainerLowest.copy(alpha = 0.8f),
                    shape = RoundedCornerShape(16.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    SelectionContainer {
                        Text(
                            text = ansiLogText(item.message, MaterialTheme.colorScheme),
                            style = MaterialTheme.typography.bodySmall.copy(fontFamily = FontFamily.Monospace, lineHeight = 18.sp),
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            fontSize = 12.sp,
                            modifier = Modifier.padding(12.dp)
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun EmptyLogState(message: String) {
    Column(modifier = Modifier.fillMaxSize(), horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.Center) {
        Icon(Icons.Rounded.History, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f))
        Spacer(Modifier.height(12.dp))
        Text(message, color = MaterialTheme.colorScheme.outline, style = MaterialTheme.typography.bodyMedium, fontWeight = FontWeight.Bold)
    }
}

// Logic components with unique names to avoid Kapt conflicts
data class MagiskLogScreenUiState(
    val loading: Boolean = true, 
    val visibleLogs: List<MagiskLogUiItem> = emptyList()
)

enum class LogDisplayFilter { ALL, ISSUES, MAGISK, SU }

enum class MagiskLogLevel(val code: Char, val shortLabel: String) {
    VERBOSE('V', "V"), DEBUG('D', "D"), INFO('I', "I"), WARN('W', "W"), ERROR('E', "E"), FATAL('F', "F"), UNKNOWN('?', "?");
    @Composable fun color() = when(this) {
        WARN -> Color(0xFFF4B400); ERROR, FATAL -> MaterialTheme.colorScheme.error
        DEBUG -> MaterialTheme.colorScheme.primary; INFO -> MaterialTheme.colorScheme.tertiary
        else -> MaterialTheme.colorScheme.outline
    }
    fun icon() = when(this) {
        WARN -> Icons.Rounded.Warning; ERROR, FATAL -> Icons.Rounded.Dangerous
        DEBUG -> Icons.Rounded.BugReport; else -> Icons.Rounded.Info
    }
    companion object { fun from(c: Char) = entries.find { it.code == c } ?: UNKNOWN }
}

data class MagiskLogUiItem(
    val id: Int,
    val timestamp: String,
    val tag: String,
    val level: MagiskLogLevel,
    val message: String,
    val raw: String,
    val pid: Int = 0,
    val tid: Int = 0
) {
    val isIssue get() = level == MagiskLogLevel.WARN || level == MagiskLogLevel.ERROR || level == MagiskLogLevel.FATAL
    val isMagisk get() = tag.contains("magisk", ignoreCase = true)
    val isSu get() = message.contains("su:", ignoreCase = true) || raw.contains("su:", ignoreCase = true)
    fun contains(q: String) = tag.contains(q, true) || message.contains(q, true) || raw.contains(q, true)
}

class MagiskLogViewModel(private val repo: LogRepository) : ViewModel() {
    private val _state = MutableStateFlow(MagiskLogScreenUiState())
    val state = _state.asStateFlow()
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages = _messages.asSharedFlow()

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            val raw = withContext(Dispatchers.IO) { repo.fetchMagiskLogs() }
            val items = withContext(Dispatchers.Default) {
                MagiskLogParser.parse(raw).mapIndexed { i, e ->
                    MagiskLogUiItem(i, e.timestamp, e.tag, MagiskLogLevel.from(e.level), e.message, e.message, e.pid, e.tid)
                }
            }
            _state.update { it.copy(loading = false, visibleLogs = items) }
        }
    }

    fun clearMagiskLogs() {
        repo.clearMagiskLogs {
            _messages.tryEmit(AppContext.getString(CoreR.string.logs_cleared))
            refresh()
        }
    }

    fun saveMagiskLog() {
        viewModelScope.launch(Dispatchers.IO) {
            val result = runCatching {
                val filename = "magisk_log_%s.log".format(System.currentTimeMillis().toTime(timeFormatStandard))
                val logFile = MediaStoreUtils.getFile(filename)
                val raw = repo.fetchMagiskLogs()
                logFile.uri.outputStream().bufferedWriter().use { it.write("---Magisk Logs---\n${Info.env.versionString}\n\n$raw") }
                logFile.toString()
            }
            withContext(Dispatchers.Main) {
                result.onSuccess { _messages.emit(AppContext.getString(CoreR.string.saved_to_path, it)) }
                    .onFailure { _messages.emit(AppContext.getString(CoreR.string.failure)) }
            }
        }
    }

    fun postExternalRwDenied() { _messages.tryEmit(AppContext.getString(CoreR.string.external_rw_permission_denied)) }

    companion object {
        private const val LOG_LINE_LIMIT = 500
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return MagiskLogViewModel(ServiceLocator.logRepo) as T
            }
        }
    }
}
