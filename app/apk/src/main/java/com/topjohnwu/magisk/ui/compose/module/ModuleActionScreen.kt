package com.topjohnwu.magisk.ui.compose.module

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.horizontalScroll
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
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.Collections
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun ModuleActionScreen(
    actionId: String,
    actionName: String,
    onBack: () -> Unit
) {
    val viewModel: ModuleActionComposeViewModel = viewModel(factory = ModuleActionComposeViewModel.Factory)
    val state by viewModel.state.collectAsState()
    val lines = viewModel.lines
    val snackbarHostState = remember { SnackbarHostState() }
    val listState = rememberLazyListState()
    val horizontalScroll = rememberScrollState()

    BackHandler(enabled = state.running) { }

    LaunchedEffect(actionId) {
        viewModel.start(actionId, actionName)
    }
    LaunchedEffect(state.message) {
        val msg = state.message ?: return@LaunchedEffect
        snackbarHostState.showSnackbar(msg)
        viewModel.consumeMessage()
    }
    LaunchedEffect(lines.size) {
        val last = lines.lastIndex
        if (last >= 0) {
            delay(30)
            listState.scrollToItem(last)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 16.dp, vertical = 12.dp)
        ) {
            Text(
                text = when {
                    state.running -> AppContext.getString(CoreR.string.running)
                    state.success -> AppContext.getString(CoreR.string.done)
                    else -> AppContext.getString(CoreR.string.failure)
                },
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold
            )
            Spacer(modifier = Modifier.height(10.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedButton(
                    onClick = { viewModel.saveLog(actionName) },
                    enabled = lines.isNotEmpty(),
                    modifier = Modifier.weight(1f)
                ) {
                    Text(text = AppContext.getString(CoreR.string.menuSaveLog))
                }
                if (!state.running) {
                    Button(
                        onClick = onBack,
                        modifier = Modifier.weight(1f)
                    ) {
                        Text(text = AppContext.getString(CoreR.string.close))
                    }
                }
            }
            Spacer(modifier = Modifier.height(12.dp))
            if (lines.isEmpty() && state.running) {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator()
                }
            } else {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .horizontalScroll(horizontalScroll)
                ) {
                    LazyColumn(
                        modifier = Modifier
                            .fillMaxSize()
                            .wrapContentWidth(),
                        contentPadding = PaddingValues(bottom = 16.dp),
                        verticalArrangement = Arrangement.spacedBy(4.dp),
                        state = listState
                    ) {
                        itemsIndexed(lines) { _, line ->
                            Text(
                                text = line,
                                fontFamily = FontFamily.Monospace,
                                style = MaterialTheme.typography.bodySmall,
                                softWrap = false
                            )
                        }
                    }
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(16.dp)
        )
    }
}

private data class ModuleActionUiState(
    val running: Boolean = false,
    val success: Boolean = false,
    val message: String? = null
)

private class ModuleActionComposeViewModel : ViewModel() {

    private val _state = MutableStateFlow(ModuleActionUiState())
    val state: StateFlow<ModuleActionUiState> = _state

    val lines = mutableStateListOf<String>()
    private val logs = Collections.synchronizedList(mutableListOf<String>())
    private val lineChannel = Channel<String>(Channel.UNLIMITED)
    private var started = false

    init {
        viewModelScope.launch(Dispatchers.Main.immediate) {
            for (line in lineChannel) {
                lines.add(line)
            }
        }
    }

    private val outItems = object : CallbackList<String>() {
        override fun onAddElement(e: String?) {
            e ?: return
            logs.add(e)
            lineChannel.trySend(e)
        }
    }

    fun start(actionId: String, actionName: String) {
        if (started) return
        started = true
        viewModelScope.launch(Dispatchers.IO) {
            _state.update { it.copy(running = true, success = false) }
            val success = runCatching {
                Shell.cmd("run_action ${shellQuote(actionId)}")
                    .to(outItems, logs)
                    .exec()
                    .isSuccess
            }.getOrDefault(false)
            withContext(Dispatchers.Main) {
                _state.update {
                    it.copy(
                        running = false,
                        success = success,
                        message = if (success) AppContext.getString(CoreR.string.done_action, actionName) else it.message
                    )
                }
            }
        }
    }

    fun saveLog(actionName: String) {
        viewModelScope.launch(Dispatchers.IO) {
            runCatching {
                val safeName = actionName.ifBlank { "module" }.replace("[^A-Za-z0-9._-]".toRegex(), "_")
                val name = "${safeName}_action_log_%s.log".format(System.currentTimeMillis().toTime(timeFormatStandard))
                val file = MediaStoreUtils.getFile(name)
                file.uri.outputStream().bufferedWriter().use { writer ->
                    synchronized(logs) {
                        logs.forEach {
                            writer.write(it)
                            writer.newLine()
                        }
                    }
                }
                file.toString()
            }.onSuccess { path ->
                _state.update { it.copy(message = path) }
            }.onFailure {
                _state.update { it.copy(message = AppContext.getString(CoreR.string.failure)) }
            }
        }
    }

    fun consumeMessage() {
        _state.update { it.copy(message = null) }
    }

    override fun onCleared() {
        lineChannel.close()
        super.onCleared()
    }

    companion object {
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return ModuleActionComposeViewModel() as T
            }
        }
    }
}

private fun shellQuote(value: String): String = "'${value.replace("'", "'\\''")}'"
