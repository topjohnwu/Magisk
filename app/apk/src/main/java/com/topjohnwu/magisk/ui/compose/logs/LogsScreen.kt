package com.topjohnwu.magisk.ui.compose.logs

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
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
import com.topjohnwu.magisk.ui.compose.RefreshOnResume
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.R as CoreR
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
import kotlinx.coroutines.withContext

@Composable
fun LogsScreen(
    viewModel: LogsComposeViewModel = viewModel(factory = LogsComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    val activity = LocalContext.current as? UIActivity<*>
    val listState = rememberLazyListState()
    val horizontalScroll = rememberScrollState()

    RefreshOnResume { viewModel.refresh() }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }
    LaunchedEffect(state.visibleLines.size) {
        val last = state.visibleLines.lastIndex
        if (last >= 0) {
            listState.animateScrollToItem(last)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 20.dp, vertical = 12.dp)
        ) {
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .weight(1f)
            ) {
                // Log Content
                if (state.loading && state.visibleLines.isEmpty()) {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        CircularProgressIndicator(strokeCap = StrokeCap.Round)
                    }
                } else if (state.visibleLines.isEmpty()) {
                    EmptyLogState(stringResource(CoreR.string.log_data_magisk_none))
                } else {
                    Column(modifier = Modifier.fillMaxSize()) {
                        Row(
                            verticalAlignment = Alignment.CenterVertically,
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 8.dp)
                        ) {
                            Surface(
                                color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f),
                                shape = RoundedCornerShape(12.dp),
                                modifier = Modifier.size(32.dp)
                            ) {
                                Icon(Icons.Rounded.Terminal, null, modifier = Modifier.padding(6.dp), tint = MaterialTheme.colorScheme.onPrimaryContainer)
                            }
                            Spacer(Modifier.width(16.dp))
                            Text(
                                text = stringResource(CoreR.string.logs),
                                style = MaterialTheme.typography.labelLarge,
                                fontWeight = FontWeight.Black,
                                letterSpacing = 1.2.sp,
                                color = MaterialTheme.colorScheme.outline
                            )
                        }
                        ElevatedCard(
                            modifier = Modifier.fillMaxSize(),
                            shape = RoundedCornerShape(24.dp),
                            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh),
                            elevation = CardDefaults.elevatedCardElevation(defaultElevation = 2.dp)
                        ) {
                            Box(
                                modifier = Modifier
                                    .fillMaxSize()
                                    .padding(16.dp)
                                    .horizontalScroll(horizontalScroll)
                            ) {
                                LazyColumn(
                                    modifier = Modifier.fillMaxSize(),
                                    state = listState,
                                    verticalArrangement = Arrangement.spacedBy(2.dp)
                                ) {
                                    itemsIndexed(state.visibleLines, key = { index, _ -> index }) { _, line ->
                                        Text(
                                            text = line,
                                            color = when {
                                                line.startsWith("!") -> MaterialTheme.colorScheme.error
                                                "error" in line.lowercase() -> MaterialTheme.colorScheme.error.copy(alpha = 0.8f)
                                                else -> MaterialTheme.colorScheme.onSurfaceVariant
                                            },
                                            fontFamily = FontFamily.Monospace,
                                            style = MaterialTheme.typography.bodySmall,
                                            softWrap = false,
                                            fontSize = 11.sp,
                                            lineHeight = 16.sp
                                        )
                                    }
                                }
                            }
                        }
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

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 120.dp)
        )
    }
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
                .height(56.dp)
                .weight(0.3f),
            shape = RoundedCornerShape(16.dp),
            color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.65f)
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
                .height(56.dp),
            shape = RoundedCornerShape(16.dp)
        ) {
            Icon(Icons.Rounded.SaveAlt, null, modifier = Modifier.size(20.dp))
            Spacer(Modifier.width(12.dp))
            Text(stringResource(CoreR.string.menuSaveLog).uppercase(), fontWeight = FontWeight.Black)
        }
    }
}

@Composable
private fun EmptyLogState(message: String) {
    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Surface(
                modifier = Modifier.size(120.dp),
                shape = RoundedCornerShape(40.dp),
                color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(Icons.Rounded.Terminal, null, modifier = Modifier.size(56.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.4f))
                }
            }
            Spacer(Modifier.height(24.dp))
            Text(message, color = MaterialTheme.colorScheme.outline, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Bold)
        }
    }
}

data class LogsUiState(
    val loading: Boolean = true,
    val visibleLines: List<String> = emptyList()
)

class LogsComposeViewModel(private val repo: LogRepository) : ViewModel() {
    private val _state = MutableStateFlow(LogsUiState())
    val state: StateFlow<LogsUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    private var refreshJob: Job? = null

    fun refresh() {
        refreshJob?.cancel()
        val hadLogs = _state.value.visibleLines.isNotEmpty()
        refreshJob = viewModelScope.launch {
            if (!hadLogs) {
                _state.update { it.copy(loading = true) }
            }
            val lines = withContext(Dispatchers.IO) {
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
                parsedLines
            }
            _state.update { it.copy(loading = false, visibleLines = lines) }
        }
    }

    fun clearMagiskLogs() {
        repo.clearMagiskLogs {
            _state.update { it.copy(visibleLines = emptyList()) }
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

    companion object {
        private const val MAX_RENDER_CHARS = 60_000
        private const val MAX_RENDER_LINES = 1_500
        private const val MAX_RENDER_LINE_CHARS = 2_000
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return LogsComposeViewModel(ServiceLocator.logRepo) as T
            }
        }
    }
}
