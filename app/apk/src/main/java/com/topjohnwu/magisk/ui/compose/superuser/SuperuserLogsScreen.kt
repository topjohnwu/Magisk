package com.topjohnwu.magisk.ui.compose.superuser

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import androidx.compose.animation.*
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.DeleteSweep
import androidx.compose.material.icons.rounded.History
import androidx.compose.material.icons.rounded.SaveAlt
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Fill
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
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.timeDateFormat
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.topjohnwu.magisk.core.R as CoreR
import kotlin.math.cos
import kotlin.math.sin

@Composable
fun SuperuserLogsScreen(
    viewModel: SuperuserLogsComposeViewModel = viewModel(factory = SuperuserLogsComposeViewModel.Factory)
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
            // Expressive Action Toolbar
            ElevatedCard(
                shape = RoundedCornerShape(28.dp),
                colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
            ) {
                Row(
                    modifier = Modifier.padding(12.dp),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    FilledTonalButton(
                        onClick = { viewModel.clearLogs() },
                        modifier = Modifier.weight(1f).height(48.dp),
                        shape = RoundedCornerShape(16.dp)
                    ) {
                        Icon(Icons.Rounded.DeleteSweep, null, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                        Text(stringResource(id = CoreR.string.menuClearLog), fontWeight = FontWeight.Bold)
                    }
                    Button(
                        onClick = {
                            activity?.withPermission(WRITE_EXTERNAL_STORAGE) { granted ->
                                if (granted) viewModel.saveLogs() else viewModel.postExternalRwDenied()
                            } ?: viewModel.saveLogs()
                        },
                        modifier = Modifier.weight(1f).height(48.dp),
                        shape = RoundedCornerShape(16.dp)
                    ) {
                        Icon(Icons.Rounded.SaveAlt, null, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                        Text(stringResource(id = CoreR.string.menuSaveLog), fontWeight = FontWeight.Bold)
                    }
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Timeline Content
            when {
                state.loading && state.items.isEmpty() -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        CircularProgressIndicator(strokeCap = StrokeCap.Round)
                    }
                }
                state.items.isEmpty() -> {
                    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Icon(Icons.Rounded.History, null, modifier = Modifier.size(64.dp), tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f))
                            Spacer(Modifier.height(16.dp))
                            Text(stringResource(id = CoreR.string.log_data_none), color = MaterialTheme.colorScheme.outline)
                        }
                    }
                }
                else -> {
                    LazyColumn(
                        modifier = Modifier.fillMaxSize(),
                        contentPadding = PaddingValues(bottom = 120.dp)
                    ) {
                        itemsIndexed(state.items, key = { _, item -> item.id }) { index, item ->
                            TimelineLogItem(index, state.items.size, item)
                        }
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
private fun TimelineLogItem(index: Int, total: Int, item: SuLogUiItem) {
    Row(modifier = Modifier.fillMaxWidth()) {
        // Timeline aesthetics
        Column(
            modifier = Modifier.width(48.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Box(
                modifier = Modifier
                    .width(2.dp)
                    .weight(1f)
                    .background(if (index == 0) Color.Transparent else MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f))
            )
            
            HexagonNode(color = if (item.allowed) Color(0xFF4CAF50) else MaterialTheme.colorScheme.error)
            
            Box(
                modifier = Modifier
                    .width(2.dp)
                    .weight(1f)
                    .background(if (index == total - 1) Color.Transparent else MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f))
            )
        }

        // Log Content Card
        ElevatedCard(
            shape = RoundedCornerShape(topEnd = 32.dp, bottomStart = 32.dp, topStart = 8.dp, bottomEnd = 8.dp),
            modifier = Modifier.padding(vertical = 8.dp, horizontal = 4.dp).weight(1f),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
        ) {
            Box {
                Icon(
                    painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier.size(80.dp).align(Alignment.TopEnd).offset(x = 20.dp, y = (-10).dp).alpha(0.05f),
                    tint = MaterialTheme.colorScheme.primary
                )
                Column(modifier = Modifier.padding(20.dp)) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(item.appName, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.ExtraBold, modifier = Modifier.weight(1f), maxLines = 1, overflow = TextOverflow.Ellipsis)
                        Surface(
                            color = (if (item.allowed) Color(0xFF4CAF50) else MaterialTheme.colorScheme.error).copy(alpha = 0.1f),
                            shape = RoundedCornerShape(8.dp)
                        ) {
                            Text(
                                text = if (item.allowed) "GRANTED" else "DENIED",
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp),
                                style = MaterialTheme.typography.labelSmall,
                                color = if (item.allowed) Color(0xFF4CAF50) else MaterialTheme.colorScheme.error,
                                fontWeight = FontWeight.Black
                            )
                        }
                    }
                    Spacer(modifier = Modifier.height(12.dp))
                    Surface(
                        color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f),
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            text = item.info,
                            modifier = Modifier.padding(12.dp),
                            style = MaterialTheme.typography.bodySmall.copy(fontFamily = FontFamily.Monospace, fontSize = 10.sp, lineHeight = 14.sp),
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun HexagonNode(color: Color) {
    Canvas(modifier = Modifier.size(24.dp)) {
        val path = Path().apply {
            val radius = size.minDimension / 2
            val centerX = size.width / 2
            val centerY = size.height / 2
            for (i in 0..5) {
                val angle = Math.toRadians((i * 60 - 30).toDouble())
                val x = centerX + radius * cos(angle).toFloat()
                val y = centerY + radius * sin(angle).toFloat()
                if (i == 0) moveTo(x, y) else lineTo(x, y)
            }
            close()
        }
        drawPath(path, color)
        drawPath(path, Color.White.copy(alpha = 0.3f), style = Fill)
    }
}

// Logic components remain identical
data class SuLogUiItem(val id: Int, val appName: String, val allowed: Boolean, val info: String)
data class SuperuserLogsUiState(val loading: Boolean = true, val items: List<SuLogUiItem> = emptyList(), val message: String? = null)

class SuperuserLogsComposeViewModel(private val repo: LogRepository) : ViewModel() {
    private val _state = MutableStateFlow(SuperuserLogsUiState())
    val state: StateFlow<SuperuserLogsUiState> = _state
    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true) }
            val logs = withContext(Dispatchers.IO) { repo.fetchSuLogs() }
            _state.update { it.copy(loading = false, items = logs.map { it.toUiItem() }) }
        }
    }
    fun clearLogs() { viewModelScope.launch { withContext(Dispatchers.IO) { repo.clearLogs() }; _state.update { it.copy(message = AppContext.getString(CoreR.string.logs_cleared)) }; refresh() } }
    fun saveLogs() {
        viewModelScope.launch(Dispatchers.IO) {
            val result = runCatching {
                val name = "superuser_log_%s.log".format(System.currentTimeMillis().toTime(timeFormatStandard))
                val logFile = MediaStoreUtils.getFile(name)
                logFile.uri.outputStream().bufferedWriter().use { writer ->
                    state.value.items.forEach { writer.write("${it.appName}\n${it.info}\n\n") }
                }
                logFile.uri.toString()
            }
            withContext(Dispatchers.Main) {
                result.onSuccess { path -> _state.update { it.copy(message = "Saved to: $path") } }
                      .onFailure { _state.update { it.copy(message = AppContext.getString(CoreR.string.failure)) } }
            }
        }
    }
    fun postExternalRwDenied() { _state.update { it.copy(message = AppContext.getString(CoreR.string.external_rw_permission_denied)) } }
    fun consumeMessage() { _state.update { it.copy(message = null) } }
    private fun SuLog.toUiItem(): SuLogUiItem = SuLogUiItem(id, appName, action >= SuPolicy.ALLOW, buildInfo())
    private fun SuLog.buildInfo(): String {
        val res = AppContext.resources
        val sb = StringBuilder()
        val date = time.toTime(timeDateFormat)
        val toUidText = res.getString(CoreR.string.target_uid, toUid)
        val fromPidText = res.getString(CoreR.string.pid, fromPid)
        sb.append("$date\n$toUidText  $fromPidText")
        if (target != -1) {
            val pid = if (target == 0) "magiskd" else target.toString()
            val targetText = res.getString(CoreR.string.target_pid, pid)
            sb.append("  $targetText")
        }
        if (context.isNotEmpty()) {
            val contextText = res.getString(CoreR.string.selinux_context, context)
            sb.append("\n$contextText")
        }
        if (gids.isNotEmpty()) {
            val gidsText = res.getString(CoreR.string.supp_group, gids)
            sb.append("\n$gidsText")
        }
        sb.append("\n$command")
        return sb.toString()
    }
    companion object { val Factory = object : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return SuperuserLogsComposeViewModel(ServiceLocator.logRepo) as T } } }
}
