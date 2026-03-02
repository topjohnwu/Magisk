package com.topjohnwu.magisk.ui.compose.logs

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
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
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
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

    LaunchedEffect(Unit) { viewModel.refresh() }
    LaunchedEffect(state.message) {
        state.message?.let { msg ->
            snackbarHostState.showSnackbar(msg)
            viewModel.consumeMessage()
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 24.dp, vertical = 12.dp)
        ) {
            // Action Toolbar
            ElevatedCard(
                shape = RoundedCornerShape(28.dp),
                colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
            ) {
                Row(
                    modifier = Modifier.padding(12.dp),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    FilledTonalButton(
                        onClick = { viewModel.clearMagiskLogs() },
                        modifier = Modifier.weight(1f).height(48.dp),
                        shape = RoundedCornerShape(16.dp)
                    ) {
                        Icon(Icons.Rounded.DeleteSweep, null, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                        Text(stringResource(CoreR.string.menuClearLog), fontWeight = FontWeight.Bold)
                    }
                    Button(
                        onClick = {
                            activity?.withPermission(WRITE_EXTERNAL_STORAGE) { if (it) viewModel.saveMagiskLog() }
                        },
                        modifier = Modifier.weight(1f).height(48.dp),
                        shape = RoundedCornerShape(16.dp)
                    ) {
                        Icon(Icons.Rounded.SaveAlt, null, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                        Text(stringResource(CoreR.string.menuSaveLog), fontWeight = FontWeight.Bold)
                    }
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Log Content
            if (state.loading) {
                Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator(strokeCap = StrokeCap.Round)
                }
            } else if (state.magiskRaw.isBlank()) {
                EmptyLogState(stringResource(CoreR.string.log_data_magisk_none))
            } else {
                SelectionContainer {
                    Box(
                        modifier = Modifier
                            .fillMaxSize()
                            .clip(RoundedCornerShape(32.dp))
                            .background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.2f))
                            .padding(20.dp)
                            .verticalScroll(rememberScrollState())
                            .horizontalScroll(rememberScrollState())
                    ) {
                        Text(
                            text = state.magiskRaw,
                            style = MaterialTheme.typography.bodySmall.copy(
                                fontFamily = FontFamily.Monospace,
                                fontSize = 10.sp,
                                lineHeight = 14.sp
                            ),
                            softWrap = false
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
private fun EmptyLogState(message: String) {
    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Icon(Icons.Rounded.Terminal, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f))
            Spacer(Modifier.height(16.dp))
            Text(message, color = MaterialTheme.colorScheme.outline, style = MaterialTheme.typography.bodyMedium)
        }
    }
}

data class LogsUiState(val loading: Boolean = true, val magiskRaw: String = "", val message: String? = null)

class LogsComposeViewModel(private val repo: LogRepository) : ViewModel() {
    private val _state = MutableStateFlow(LogsUiState())
    val state: StateFlow<LogsUiState> = _state

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            val raw = withContext(Dispatchers.IO) { repo.fetchMagiskLogs() }
            _state.update { it.copy(loading = false, magiskRaw = raw) }
        }
    }

    fun clearMagiskLogs() {
        repo.clearMagiskLogs {
            postMessageText("Logs cleared")
            refresh()
        }
    }

    fun saveMagiskLog() {
        viewModelScope.launch(Dispatchers.IO) {
            val result = runCatching {
                val filename = "magisk_log_%s.log".format(System.currentTimeMillis().toTime(timeFormatStandard))
                val logFile = MediaStoreUtils.getFile(filename)
                logFile.uri.outputStream().bufferedWriter().use { it.write("---Magisk Logs---\n${Info.env.versionString}\n\n${_state.value.magiskRaw}") }
                logFile.toString()
            }
            withContext(Dispatchers.Main) {
                result.onSuccess { postMessageText("Saved to: $it") }.onFailure { postMessageText("Save failed") }
            }
        }
    }

    private fun postMessageText(text: String) { _state.update { it.copy(message = text) } }
    fun consumeMessage() { _state.update { it.copy(message = null) } }

    companion object {
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return LogsComposeViewModel(ServiceLocator.logRepo) as T
            }
        }
    }
}
