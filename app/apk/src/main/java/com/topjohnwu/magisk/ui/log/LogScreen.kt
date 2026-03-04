package com.topjohnwu.magisk.ui.log

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
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
import com.topjohnwu.magisk.ui.RefreshOnResume
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@Composable
fun LogsScreen(
    viewModel: LogsComposeViewModel = viewModel(factory = LogsComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    val activity = LocalContext.current as? UIActivity<*>
    val listState = rememberLazyListState()
    val scope = rememberCoroutineScope()
    
    var filter by remember { mutableStateOf(LogDisplayFilter.ALL) }
    var searchQuery by remember { mutableStateOf("") }
    var isSearchActive by remember { mutableStateOf(false) }

    val filteredLogs = remember(state.visibleLogs, filter, searchQuery) {
        val base = when (filter) {
            LogDisplayFilter.ALL -> state.visibleLogs
            LogDisplayFilter.ISSUES -> state.visibleLogs.filter {
                it.level == MagiskLogLevel.WARN || it.level == MagiskLogLevel.ERROR || it.level == MagiskLogLevel.FATAL
            }
            LogDisplayFilter.MAGISK -> state.visibleLogs.filter { it.tag.contains("magisk", ignoreCase = true) }
            LogDisplayFilter.SU -> state.visibleLogs.filter {
                it.message.startsWith("su:", ignoreCase = true) || it.raw.startsWith("su:", ignoreCase = true)
            }
        }
        if (searchQuery.isEmpty()) base
        else base.filter { 
            it.message.contains(searchQuery, ignoreCase = true) || 
            it.tag.contains(searchQuery, ignoreCase = true) ||
            it.raw.contains(searchQuery, ignoreCase = true)
        }
    }

    val canScrollDown by remember { derivedStateOf { listState.canScrollForward } }

    RefreshOnResume { viewModel.refresh() }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }
    LaunchedEffect(filteredLogs.lastOrNull()?.id) {
        val last = filteredLogs.lastIndex
        if (last >= 0 && !listState.isScrollInProgress) {
            listState.animateScrollToItem(last)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 16.dp, vertical = 8.dp)
        ) {
            if (state.loading && state.visibleLogs.isEmpty()) {
                Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator(strokeCap = StrokeCap.Round)
                }
            } else if (state.visibleLogs.isEmpty()) {
                EmptyLogState(stringResource(CoreR.string.log_data_magisk_none))
            } else {
                val issueCount = state.visibleLogs.count {
                    it.level == MagiskLogLevel.WARN || it.level == MagiskLogLevel.ERROR || it.level == MagiskLogLevel.FATAL
                }
                val magiskCount = state.visibleLogs.count { it.tag.contains("magisk", ignoreCase = true) }
                val suCount = state.visibleLogs.count {
                    it.message.startsWith("su:", ignoreCase = true) || it.raw.startsWith("su:", ignoreCase = true)
                }

                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.fillMaxWidth().height(56.dp)
                ) {
                    AnimatedContent(
                        targetState = isSearchActive,
                        transitionSpec = {
                            fadeIn() togetherWith fadeOut()
                        },
                        modifier = Modifier.weight(1f),
                        label = "search_transition"
                    ) { active ->
                        if (active) {
                            TextField(
                                value = searchQuery,
                                onValueChange = { searchQuery = it },
                                modifier = Modifier.fillMaxWidth(),
                                placeholder = { Text("Cerca nei log...", fontSize = 14.sp) },
                                leadingIcon = { Icon(Icons.Rounded.Search, null, modifier = Modifier.size(20.dp)) },
                                trailingIcon = {
                                    IconButton(onClick = { 
                                        isSearchActive = false
                                        searchQuery = ""
                                    }) {
                                        Icon(Icons.Rounded.Close, null)
                                    }
                                },
                                colors = TextFieldDefaults.colors(
                                    focusedContainerColor = MaterialTheme.colorScheme.surfaceContainer,
                                    unfocusedContainerColor = MaterialTheme.colorScheme.surfaceContainer,
                                    focusedIndicatorColor = Color.Transparent,
                                    unfocusedIndicatorColor = Color.Transparent
                                ),
                                shape = RoundedCornerShape(12.dp),
                                singleLine = true,
                                textStyle = MaterialTheme.typography.bodyMedium
                            )
                        } else {
                            LogsFilterBar(
                                selected = filter,
                                total = state.visibleLogs.size,
                                issues = issueCount,
                                magisk = magiskCount,
                                su = suCount,
                                onSelected = { filter = it }
                            )
                        }
                    }
                    
                    if (!isSearchActive) {
                        Spacer(Modifier.width(8.dp))
                        Surface(
                            onClick = { isSearchActive = true },
                            color = MaterialTheme.colorScheme.secondaryContainer.copy(alpha = 0.4f),
                            shape = RoundedCornerShape(12.dp),
                            modifier = Modifier.size(48.dp)
                        ) {
                            Box(contentAlignment = Alignment.Center) {
                                Icon(Icons.Rounded.Search, null, tint = MaterialTheme.colorScheme.onSecondaryContainer)
                            }
                        }
                    }
                }
                
                Spacer(Modifier.height(12.dp))

                LazyColumn(
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    state = listState,
                    contentPadding = PaddingValues(bottom = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    items(filteredLogs, key = { it.id }) { item ->
                        CutePrettyLogCard(item = item)
                    }
                }
            }

            Spacer(modifier = Modifier.height(12.dp))
            LogsActionButtons(
                onClear = { viewModel.clearMagiskLogs() },
                onSave = {
                    activity?.withPermission(WRITE_EXTERNAL_STORAGE) {
                        if (it) viewModel.saveMagiskLog() else viewModel.postExternalRwDenied()
                    } ?: viewModel.saveMagiskLog()
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .navigationBarsPadding()
                    .padding(bottom = 8.dp)
            )
        }

        // FAB to scroll to bottom
        AnimatedVisibility(
            visible = canScrollDown,
            enter = scaleIn() + fadeIn(),
            exit = scaleOut() + fadeOut(),
            modifier = Modifier
                .align(Alignment.BottomEnd)
                .padding(bottom = 90.dp, end = 20.dp)
        ) {
            FloatingActionButton(
                onClick = {
                    scope.launch {
                        if (filteredLogs.isNotEmpty()) {
                            listState.animateScrollToItem(filteredLogs.lastIndex)
                        }
                    }
                },
                containerColor = MaterialTheme.colorScheme.primaryContainer,
                contentColor = MaterialTheme.colorScheme.onPrimaryContainer,
                shape = CircleShape,
                modifier = Modifier.size(48.dp)
            ) {
                Icon(Icons.Rounded.ArrowDownward, null)
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 120.dp)
        )
    }
}

@Composable
private fun CutePrettyLogCard(item: MagiskLogUiItem) {
    val accentColor = item.level.color()
    val hasMeta = item.timestamp != null || item.pid != null || item.tid != null
    
    Card(
        shape = RoundedCornerShape(32.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
        ),
        modifier = Modifier.fillMaxWidth()
    ) {
        Box {
            // Background Watermark
            Icon(
                painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                contentDescription = null,
                modifier = Modifier
                    .size(100.dp)
                    .align(Alignment.TopEnd)
                    .offset(x = 20.dp, y = (-15).dp)
                    .alpha(0.04f),
                tint = accentColor
            )

            Column(modifier = Modifier.padding(16.dp)) {
                // HEADER ROW: [Badge] Tag
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    // 1. Symbol (Level Badge) - Perfect Circle
                    Surface(
                        shape = CircleShape,
                        color = accentColor.copy(alpha = 0.25f),
                        modifier = Modifier.size(32.dp)
                    ) {
                        Box(contentAlignment = Alignment.Center) {
                            Text(
                                text = item.level.shortLabel,
                                color = accentColor,
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.Black
                            )
                        }
                    }

                    Spacer(Modifier.width(10.dp))

                    // 2. Tag (Nome)
                    Text(
                        text = item.tag,
                        style = MaterialTheme.typography.labelLarge,
                        color = accentColor,
                        fontWeight = FontWeight.ExtraBold,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                        modifier = Modifier.weight(1f, fill = false)
                    )
                }

                if (hasMeta) {
                    Spacer(Modifier.height(8.dp))
                    val metaText = buildList {
                        item.timestamp?.let { add(it) }
                        item.pid?.let { add("pid:$it") }
                        item.tid?.let { add("tid:$it") }
                    }.joinToString("   ")

                    Text(
                        text = metaText,
                        style = MaterialTheme.typography.labelSmall.copy(fontFamily = FontFamily.Monospace),
                        color = MaterialTheme.colorScheme.outline
                    )
                }

                Spacer(Modifier.height(12.dp))

                // Message area - New Line (LOG A CAPO) - Cozy Bubble
                Surface(
                    color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.04f),
                    shape = RoundedCornerShape(20.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    SelectionContainer {
                        Text(
                            text = item.message.ifBlank { item.raw },
                            modifier = Modifier.padding(16.dp),
                            style = MaterialTheme.typography.bodySmall.copy(
                                fontFamily = FontFamily.Monospace,
                                lineHeight = 20.sp
                            ),
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            fontSize = 12.sp
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun LogsFilterBar(
    selected: LogDisplayFilter,
    total: Int,
    issues: Int,
    magisk: Int,
    su: Int,
    onSelected: (LogDisplayFilter) -> Unit
) {
    val scroll = rememberScrollState()
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .horizontalScroll(scroll),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        LogFilterChip(
            selected = selected == LogDisplayFilter.ALL,
            label = "Tutti",
            badge = total,
            onClick = { onSelected(LogDisplayFilter.ALL) }
        )
        LogFilterChip(
            selected = selected == LogDisplayFilter.ISSUES,
            label = "Problemi",
            badge = issues,
            onClick = { onSelected(LogDisplayFilter.ISSUES) }
        )
        LogFilterChip(
            selected = selected == LogDisplayFilter.MAGISK,
            label = "Magisk",
            badge = magisk,
            onClick = { onSelected(LogDisplayFilter.MAGISK) }
        )
        LogFilterChip(
            selected = selected == LogDisplayFilter.SU,
            label = "su",
            badge = su,
            onClick = { onSelected(LogDisplayFilter.SU) }
        )
    }
}

@Composable
private fun LogFilterChip(
    selected: Boolean,
    label: String,
    badge: Int,
    onClick: () -> Unit
) {
    FilterChip(
        selected = selected,
        onClick = onClick,
        label = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(label)
                if (badge > 0) {
                    Spacer(Modifier.width(6.dp))
                    Surface(
                        color = if (selected) MaterialTheme.colorScheme.onSecondaryContainer.copy(alpha = 0.2f) 
                                else MaterialTheme.colorScheme.secondaryContainer,
                        shape = RoundedCornerShape(10.dp)
                    ) {
                        Text(
                            text = badge.toString(),
                            modifier = Modifier.padding(horizontal = 6.dp, vertical = 1.dp),
                            style = MaterialTheme.typography.labelSmall,
                            fontWeight = FontWeight.Bold
                        )
                    }
                }
            }
        },
        shape = RoundedCornerShape(12.dp)
    )
}

@Composable
private fun LogsActionButtons(
    onClear: () -> Unit,
    onSave: () -> Unit,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier,
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Surface(
            onClick = onClear,
            modifier = Modifier
                .height(52.dp)
                .weight(0.3f),
            shape = RoundedCornerShape(14.dp),
            color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.7f)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(
                    imageVector = Icons.Rounded.DeleteSweep,
                    contentDescription = stringResource(CoreR.string.menuClearLog),
                    tint = MaterialTheme.colorScheme.onErrorContainer
                )
            }
        }

        Button(
            onClick = onSave,
            modifier = Modifier
                .weight(1f)
                .height(52.dp),
            shape = RoundedCornerShape(14.dp)
        ) {
            Icon(Icons.Rounded.SaveAlt, null, modifier = Modifier.size(20.dp))
            Spacer(Modifier.width(12.dp))
            Text(stringResource(CoreR.string.menuSaveLog).uppercase(), fontWeight = FontWeight.Bold)
        }
    }
}

@Composable
private fun EmptyLogState(message: String) {
    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Surface(
                modifier = Modifier.size(100.dp),
                shape = RoundedCornerShape(32.dp),
                color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(Icons.Rounded.Terminal, null, modifier = Modifier.size(48.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.4f))
                }
            }
            Spacer(Modifier.height(20.dp))
            Text(message, color = MaterialTheme.colorScheme.outline, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Bold)
        }
    }
}

data class LogsUiState(
    val loading: Boolean = true,
    val visibleLogs: List<MagiskLogUiItem> = emptyList()
)

enum class LogDisplayFilter {
    ALL,
    ISSUES,
    MAGISK,
    SU
}

enum class MagiskLogLevel {
    VERBOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    UNKNOWN;

    val shortLabel: String
        get() = when (this) {
            VERBOSE -> "V"
            DEBUG -> "D"
            INFO -> "I"
            WARN -> "W"
            ERROR -> "E"
            FATAL -> "F"
            UNKNOWN -> "?"
        }

    @Composable
    fun color(): Color = when (this) {
        VERBOSE -> MaterialTheme.colorScheme.outline
        DEBUG -> MaterialTheme.colorScheme.primary
        INFO -> MaterialTheme.colorScheme.tertiary
        WARN -> Color(0xFFF4B400)
        ERROR -> MaterialTheme.colorScheme.error
        FATAL -> MaterialTheme.colorScheme.error
        UNKNOWN -> MaterialTheme.colorScheme.outline
    }

    fun icon(): ImageVector = when (this) {
        VERBOSE -> Icons.Rounded.Article
        DEBUG -> Icons.Rounded.BugReport
        INFO -> Icons.Rounded.Info
        WARN -> Icons.Rounded.Warning
        ERROR -> Icons.Rounded.Error
        FATAL -> Icons.Rounded.Dangerous
        UNKNOWN -> Icons.Rounded.HelpOutline
    }
}

data class MagiskLogUiItem(
    val id: Int,
    val raw: String,
    val message: String,
    val level: MagiskLogLevel,
    val timestamp: String?,
    val tag: String,
    val pid: Int?,
    val tid: Int?
)

class LogsComposeViewModel(private val repo: LogRepository) : ViewModel() {
    private val _state = MutableStateFlow(LogsUiState())
    val state: StateFlow<LogsUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    private var refreshJob: Job? = null

    fun refresh() {
        refreshJob?.cancel()
        val hadLogs = _state.value.visibleLogs.isNotEmpty()
        refreshJob = viewModelScope.launch {
            if (!hadLogs) {
                _state.update { it.copy(loading = true) }
            }
            val logItems = withContext(Dispatchers.IO) {
                val fetched = repo.fetchMagiskLogs()
                val visibleRaw = fetched.takeLast(MAX_RENDER_CHARS)
                val parsedLines = if (visibleRaw.isBlank()) {
                    emptyList()
                } else {
                    visibleRaw
                        .split('\n')
                        .let { list -> if (list.size > MAX_RENDER_LINES) list.takeLast(MAX_RENDER_LINES) else list }
                        .map { line -> if (line.length > MAX_RENDER_LINE_CHARS) line.take(MAX_RENDER_LINE_CHARS) else line }
                }
                parsedLines.mapIndexedNotNull { index, line -> parseLogLine(index, line) }
            }
            _state.update { it.copy(loading = false, visibleLogs = logItems) }
        }
    }

    fun clearMagiskLogs() {
        repo.clearMagiskLogs {
            _state.update { it.copy(visibleLogs = emptyList()) }
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
                logFile.uri.outputStream().bufferedWriter().use {
                    it.write("---Magisk Logs---\n${Info.env.versionString}\n\n$raw")
                }
                logFile.toString()
            }
            withContext(Dispatchers.Main) {
                result.onSuccess { _messages.emit(AppContext.getString(CoreR.string.saved_to_path, it)) }
                    .onFailure { _messages.emit(AppContext.getString(CoreR.string.failure)) }
            }
        }
    }

    fun postExternalRwDenied() { _messages.tryEmit(AppContext.getString(CoreR.string.external_rw_permission_denied)) }

    private fun parseLogLine(index: Int, line: String): MagiskLogUiItem? {
        val raw = line.trimEnd()
        if (raw.isBlank()) {
            return null
        }

        fullLogcatRegex.matchEntire(raw)?.let { match ->
            return MagiskLogUiItem(
                id = index,
                raw = raw,
                message = match.groupValues[6].trim(),
                level = parseLevelToken(match.groupValues[4]),
                timestamp = match.groupValues[1],
                tag = match.groupValues[5].trim().ifEmpty { "Log" },
                pid = match.groupValues[2].toIntOrNull(),
                tid = match.groupValues[3].toIntOrNull()
            )
        }

        simpleLogcatRegex.matchEntire(raw)?.let { match ->
            val timestamp = match.groupValues[1].ifBlank { null }
            return MagiskLogUiItem(
                id = index,
                raw = raw,
                message = match.groupValues[4].trim(),
                level = parseLevelToken(match.groupValues[2]),
                timestamp = timestamp,
                tag = match.groupValues[3].trim().ifEmpty { "Log" },
                pid = null,
                tid = null
            )
        }

        val lower = raw.lowercase()
        val level = when {
            raw.startsWith("!") || "error" in lower || "fatal" in lower || "fail" in lower -> MagiskLogLevel.ERROR
            "warn" in lower -> MagiskLogLevel.WARN
            "debug" in lower -> MagiskLogLevel.DEBUG
            else -> MagiskLogLevel.INFO
        }

        return MagiskLogUiItem(
            id = index,
            raw = raw,
            message = raw,
            level = level,
            timestamp = null,
            tag = if ("magisk" in lower) "Magisk" else "Log",
            pid = null,
            tid = null
        )
    }

    private fun parseLevelToken(token: String): MagiskLogLevel {
        return when (token.uppercase()) {
            "V" -> MagiskLogLevel.VERBOSE
            "D" -> MagiskLogLevel.DEBUG
            "I" -> MagiskLogLevel.INFO
            "W" -> MagiskLogLevel.WARN
            "E" -> MagiskLogLevel.ERROR
            "F", "A" -> MagiskLogLevel.FATAL
            else -> MagiskLogLevel.UNKNOWN
        }
    }

    companion object {
        private const val MAX_RENDER_CHARS = 60_000
        private const val MAX_RENDER_LINES = 1_500
        private const val MAX_RENDER_LINE_CHARS = 2_000
        private const val TIMESTAMP_REGEX = "(?:\\d{2}-\\d{2}|\\d{4}-\\d{2}-\\d{2})\\s+\\d{2}:\\d{2}:\\d{2}(?:\\.\\d{3,6})?"
        private val fullLogcatRegex = Regex("""^\s*($TIMESTAMP_REGEX)\s+(\d+)\s+(\d+)\s+([VDIWEAF])\s+([^:]+?)\s*:\s*(.*)$""")
        private val simpleLogcatRegex = Regex("""^\s*(?:($TIMESTAMP_REGEX)\s+)?([VDIWEAF])\s+([^:]+?)\s*:\s*(.*)$""")
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return LogsComposeViewModel(ServiceLocator.logRepo) as T
            }
        }
    }
}
