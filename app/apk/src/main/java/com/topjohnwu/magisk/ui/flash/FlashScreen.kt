package com.topjohnwu.magisk.ui.flash

import android.net.Uri
import android.os.Build
import androidx.activity.compose.BackHandler
import androidx.compose.animation.*
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.spring
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.Notes
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.ui.RouteProcessTopBarState
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.CallbackList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.Collections
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun FlashScreen(
    action: String,
    uriArg: String?,
    onTitleStateChange: (String?, String?, RouteProcessTopBarState) -> Unit = { _, _, _ -> },
    onBack: () -> Unit
) {
    val viewModel: FlashComposeViewModel = viewModel(factory = FlashComposeViewModel.Factory)
    val state by viewModel.state.collectAsState()
    val lines = viewModel.lines
    val snackbarHostState = remember { SnackbarHostState() }
    val listState = rememberLazyListState()
    val horizontalScroll = rememberScrollState()
    val parsedUri = uriArg
        ?.takeIf { it.isNotBlank() }
        ?.let(::parseNavigationUriArg)
    var hasStarted by remember(action, uriArg) { mutableStateOf(false) }

    BackHandler(enabled = state.running) { }

    LaunchedEffect(action, parsedUri) {
        viewModel.start(action, parsedUri)
    }
    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }
    LaunchedEffect(lines.size) {
        val last = lines.lastIndex
        if (last >= 0) {
            delay(30)
            listState.animateScrollToItem(last)
        }
    }
    LaunchedEffect(state.running, state.success, lines.size) {
        if (state.running || lines.isNotEmpty()) {
            hasStarted = true
        }
        val title = AppContext.getString(CoreR.string.flash_screen_title)
        val subtitle = when {
            state.running -> AppContext.getString(CoreR.string.flashing)
            !hasStarted -> null
            state.success -> AppContext.getString(CoreR.string.done)
            else -> AppContext.getString(CoreR.string.failure)
        }
        onTitleStateChange(
            title,
            subtitle,
            RouteProcessTopBarState(
                running = state.running,
                success = state.success,
                hasResult = hasStarted && !state.running
            )
        )
    }
    DisposableEffect(Unit) {
        onDispose { onTitleStateChange(null, null, RouteProcessTopBarState()) }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 20.dp, vertical = 12.dp),
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            LogContainer(
                lines = lines,
                listState = listState,
                horizontalScroll = horizontalScroll,
                modifier = Modifier.weight(1f)
            )

            ActionButtons(
                state = state,
                hasLogs = lines.isNotEmpty(),
                onSaveLog = viewModel::saveLog,
                onReboot = viewModel::rebootNow,
                onClose = onBack
            )
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(bottom = 110.dp)
        )
    }
}

@Composable
private fun FlashHeader(state: FlashUiState) {
    ElevatedCard(
        shape = RoundedCornerShape(28.dp),
        colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh),
        elevation = CardDefaults.elevatedCardElevation(defaultElevation = 2.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth().padding(20.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(20.dp)
        ) {
            Box(contentAlignment = Alignment.Center) {
                if (state.running) {
                    CircularProgressIndicator(
                        modifier = Modifier.size(48.dp),
                        strokeWidth = 4.dp,
                        color = MaterialTheme.colorScheme.primary,
                        trackColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f)
                    )
                } else {
                    Surface(
                        color = if (state.success) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.errorContainer,
                        shape = CircleShape,
                        modifier = Modifier.size(48.dp)
                    ) {
                        Icon(
                            imageVector = if (state.success) Icons.Rounded.CheckCircle else Icons.Rounded.Error,
                            contentDescription = null,
                            modifier = Modifier.padding(8.dp),
                            tint = if (state.success) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onErrorContainer
                        )
                    }
                }
            }

            Column(modifier = Modifier.weight(1f)) {
                AnimatedContent(
                    targetState = when {
                        state.running -> AppContext.getString(CoreR.string.flashing)
                        state.success -> AppContext.getString(CoreR.string.done)
                        else -> AppContext.getString(CoreR.string.failure)
                    },
                    transitionSpec = {
                        (slideInVertically { height -> height } + fadeIn() togetherWith
                         slideOutVertically { height -> -height } + fadeOut()).using(
                            SizeTransform(clip = false)
                        )
                    },
                    label = "flashTitleAnimation"
                ) { targetText ->
                    Text(
                        text = targetText,
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Black,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                }
                if (state.running) {
                    Text(
                        text = AppContext.getString(CoreR.string.flash_in_progress_subtitle),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

@Composable
private fun LogContainer(
    lines: List<String>,
    listState: androidx.compose.foundation.lazy.LazyListState,
    horizontalScroll: androidx.compose.foundation.ScrollState,
    modifier: Modifier = Modifier
) {
    Column(modifier = modifier) {
        Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(horizontal = 8.dp, vertical = 8.dp)) {
            Icon(Icons.Rounded.Terminal, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.primary)
            Spacer(Modifier.width(12.dp))
            Text(text = AppContext.getString(CoreR.string.logs), style = MaterialTheme.typography.labelLarge, fontWeight = FontWeight.Black, letterSpacing = 1.2.sp, color = MaterialTheme.colorScheme.outline)
        }

        ElevatedCard(
            modifier = Modifier.fillMaxSize(),
            shape = RoundedCornerShape(24.dp),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
        ) {
            Box(modifier = Modifier.fillMaxSize().padding(16.dp).horizontalScroll(horizontalScroll)) {
                if (lines.isEmpty()) {
                    Text(text = AppContext.getString(CoreR.string.waiting_for_logs), color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.5f), style = MaterialTheme.typography.bodySmall, fontFamily = FontFamily.Monospace)
                } else {
                    LazyColumn(modifier = Modifier.fillMaxSize(), state = listState, verticalArrangement = Arrangement.spacedBy(2.dp)) {
                        itemsIndexed(lines) { _, line ->
                            Text(text = line, color = if (line.startsWith("!")) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurfaceVariant, fontFamily = FontFamily.Monospace, style = MaterialTheme.typography.bodySmall, softWrap = false, fontSize = 11.sp)
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun ActionButtons(
    state: FlashUiState,
    hasLogs: Boolean,
    onSaveLog: () -> Unit,
    onReboot: () -> Unit,
    onClose: () -> Unit
) {
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 8.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
        OutlinedButton(
            onClick = onSaveLog,
            enabled = hasLogs,
            modifier = Modifier.weight(1f).height(56.dp),
            shape = RoundedCornerShape(16.dp),
            border = ButtonDefaults.outlinedButtonBorder(hasLogs).copy(width = 1.dp)
        ) {
            Icon(Icons.Rounded.Save, null, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(8.dp))
            Text(AppContext.getString(CoreR.string.menuSaveLog), fontWeight = FontWeight.Bold)
        }

        AnimatedContent(targetState = !state.running, modifier = Modifier.weight(1f), label = "actionBtn") { isFinished ->
            if (isFinished) {
                if (state.showReboot) {
                    Button(
                        onClick = onReboot,
                        modifier = Modifier.fillMaxWidth().height(56.dp),
                        shape = RoundedCornerShape(16.dp),
                        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)
                    ) {
                        Icon(Icons.Rounded.RestartAlt, null)
                        Spacer(Modifier.width(8.dp))
                        Text(AppContext.getString(CoreR.string.reboot), fontWeight = FontWeight.Black)
                    }
                } else {
                    Button(onClick = onClose, modifier = Modifier.fillMaxWidth().height(56.dp), shape = RoundedCornerShape(16.dp)) {
                        Text(AppContext.getString(CoreR.string.close), fontWeight = FontWeight.Black)
                    }
                }
            } else {
                Button(onClick = { }, enabled = false, modifier = Modifier.fillMaxWidth().height(56.dp), shape = RoundedCornerShape(16.dp)) {
                    Text(AppContext.getString(CoreR.string.running), fontWeight = FontWeight.Bold)
                }
            }
        }
    }
}

private data class FlashUiState(val running: Boolean = false, val success: Boolean = false, val showReboot: Boolean = Info.isRooted)

private class FlashComposeViewModel : ViewModel() {
    private val _state = MutableStateFlow(FlashUiState())
    val state: StateFlow<FlashUiState> = _state
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()
    val lines = mutableStateListOf<String>()
    private val logs = Collections.synchronizedList(mutableListOf<String>())
    private val lineChannel = Channel<String>(Channel.UNLIMITED)
    private var started = false

    init {
        viewModelScope.launch(Dispatchers.Main.immediate) { for (line in lineChannel) { lines.add(line) } }
    }

    private val outItems = object : CallbackList<String>() {
        override fun onAddElement(e: String?) {
            e ?: return
            logs.add(e)
            lineChannel.trySend(e)
        }
    }

    fun start(action: String, uri: Uri?) {
        if (started) return
        started = true
        viewModelScope.launch {
            _state.update { it.copy(running = true, success = false, showReboot = Info.isRooted) }
            if (requiresRoot(action)) {
                val hasRoot = withContext(Dispatchers.IO) { Shell.getShell().isRoot }
                if (!hasRoot) {
                    outItems.add("! ${AppContext.getString(CoreR.string.root_required_operation)}")
                    _state.update { it.copy(running = false, success = false, showReboot = false) }
                    _messages.emit(AppContext.getString(CoreR.string.failure))
                    return@launch
                }
            }
            val result = when (action) {
                Const.Value.FLASH_ZIP -> if (uri == null) false else FlashZip(uri, outItems, logs).exec()
                Const.Value.UNINSTALL -> { _state.update { it.copy(showReboot = false) }; MagiskInstaller.Uninstall(outItems, logs).exec() }
                Const.Value.FLASH_MAGISK -> if (Info.isEmulator) MagiskInstaller.Emulator(outItems, logs).exec() else MagiskInstaller.Direct(outItems, logs).exec()
                Const.Value.FLASH_INACTIVE_SLOT -> { _state.update { it.copy(showReboot = false) }; MagiskInstaller.SecondSlot(outItems, logs).exec() }
                Const.Value.PATCH_FILE -> if (uri == null) false else { _state.update { it.copy(showReboot = false) }; MagiskInstaller.Patch(uri, outItems, logs).exec() }
                else -> false
            }
            _state.update { it.copy(running = false, success = result) }
        }
    }

    fun saveLog() {
        viewModelScope.launch(Dispatchers.IO) {
            runCatching {
                val name = "magisk_install_log_%s.log".format(System.currentTimeMillis().toTime(timeFormatStandard))
                val file = MediaStoreUtils.getFile(name)
                file.uri.outputStream().bufferedWriter().use { writer -> synchronized(logs) { logs.forEach { writer.write(it); writer.newLine() } } }
                file.toString()
            }.onSuccess { path -> _messages.emit(path) }.onFailure { _messages.emit(AppContext.getString(CoreR.string.failure)) }
        }
    }

    fun rebootNow() { reboot() }
    override fun onCleared() { lineChannel.close(); super.onCleared() }
    private fun requiresRoot(action: String): Boolean = action == Const.Value.FLASH_ZIP || action == Const.Value.UNINSTALL || action == Const.Value.FLASH_MAGISK || action == Const.Value.FLASH_INACTIVE_SLOT
    companion object { val Factory = object : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return FlashComposeViewModel() as T } } }
}

private fun parseNavigationUriArg(rawArg: String): Uri? {
    val rawParsed = runCatching { Uri.parse(rawArg) }.getOrNull()
    if (rawParsed?.scheme == "content" || rawParsed?.scheme == "file") {
        return rawParsed
    }
    val decoded = runCatching { Uri.decode(rawArg) }.getOrNull() ?: return rawParsed
    val decodedParsed = runCatching { Uri.parse(decoded) }.getOrNull()
    return when (decodedParsed?.scheme) {
        "content", "file" -> decodedParsed
        else -> rawParsed
    }
}
