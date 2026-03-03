package com.topjohnwu.magisk.ui.log

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.nestedscroll.NestedScrollConnection
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.topjohnwu.magisk.core.ktx.timeDateFormat
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.model.su.SuLog
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.TabRow
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun LogScreen(viewModel: LogViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    var selectedTab by rememberSaveable { mutableIntStateOf(0) }
    val tabTitles = listOf(
        stringResource(CoreR.string.superuser),
        stringResource(CoreR.string.magisk)
    )
    val scrollBehavior = MiuixScrollBehavior()

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.logs),
                scrollBehavior = scrollBehavior
            )
        }
    ) { padding ->
        Column(modifier = Modifier
            .fillMaxSize()
            .padding(padding)
        ) {
            TabRow(
                tabs = tabTitles,
                selectedTabIndex = selectedTab,
                onTabSelected = { selectedTab = it },
                modifier = Modifier.fillMaxWidth()
            )

            if (uiState.loading) {
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    CircularProgressIndicator()
                }
            } else {
                when (selectedTab) {
                    0 -> SuLogTab(
                        logs = uiState.suLogs,
                        onClear = { viewModel.clearLog() },
                        nestedScrollConnection = scrollBehavior.nestedScrollConnection
                    )
                    1 -> MagiskLogTab(
                        entries = uiState.magiskLogEntries,
                        onSave = { viewModel.saveMagiskLog() },
                        onClear = { viewModel.clearMagiskLog() },
                        nestedScrollConnection = scrollBehavior.nestedScrollConnection
                    )
                }
            }
        }
    }
}

@Composable
private fun SuLogTab(logs: List<SuLog>, onClear: () -> Unit, nestedScrollConnection: NestedScrollConnection) {
    Column(modifier = Modifier.fillMaxSize()) {
        if (logs.isEmpty()) {
            Box(
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth(),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.log_data_none),
                    style = MiuixTheme.textStyles.body1,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                )
            }
        } else {
            LazyColumn(
                modifier = Modifier
                    .weight(1f)
                    .nestedScroll(nestedScrollConnection)
                    .padding(horizontal = 12.dp),
                contentPadding = PaddingValues(bottom = 88.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                item { Spacer(Modifier.height(4.dp)) }
                items(logs, key = { it.id }) { log ->
                    SuLogCard(log)
                }
                item { Spacer(Modifier.height(4.dp)) }
            }
        }

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.End
        ) {
            TextButton(
                text = stringResource(CoreR.string.menuClearLog),
                onClick = onClear
            )
        }
    }
}

@Composable
private fun SuLogCard(log: SuLog) {
    val res = LocalContext.current.resources
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = log.appName,
                    style = MiuixTheme.textStyles.body1,
                )
                val allowed = log.action >= 2
                Text(
                    text = if (allowed) stringResource(CoreR.string.grant) else stringResource(CoreR.string.deny),
                    style = MiuixTheme.textStyles.body2,
                    color = if (allowed) MiuixTheme.colorScheme.onSurfaceVariantSummary
                        else MiuixTheme.colorScheme.primary
                )
            }

            Spacer(Modifier.height(4.dp))

            val info = buildString {
                val date = log.time.toTime(timeDateFormat)
                val toUid = res.getString(CoreR.string.target_uid, log.toUid)
                val fromPid = res.getString(CoreR.string.pid, log.fromPid)
                append("$date\n$toUid  $fromPid")
                if (log.target != -1) {
                    val pid = if (log.target == 0) "magiskd" else log.target.toString()
                    val target = res.getString(CoreR.string.target_pid, pid)
                    append("  $target")
                }
                if (log.context.isNotEmpty()) {
                    val context = res.getString(CoreR.string.selinux_context, log.context)
                    append("\n$context")
                }
                if (log.gids.isNotEmpty()) {
                    val gids = res.getString(CoreR.string.supp_group, log.gids)
                    append("\n$gids")
                }
                append("\n${log.command}")
            }

            Text(
                text = info,
                style = MiuixTheme.textStyles.body2,
                color = MiuixTheme.colorScheme.onSurfaceVariantSummary
            )
        }
    }
}

@Composable
private fun MagiskLogTab(
    entries: List<MagiskLogEntry>,
    onSave: () -> Unit,
    onClear: () -> Unit,
    nestedScrollConnection: NestedScrollConnection
) {
    Column(modifier = Modifier.fillMaxSize()) {
        if (entries.isEmpty()) {
            Box(
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth(),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.log_data_magisk_none),
                    style = MiuixTheme.textStyles.body1,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                )
            }
        } else {
            val listState = rememberLazyListState(initialFirstVisibleItemIndex = entries.size - 1)
            LazyColumn(
                state = listState,
                modifier = Modifier
                    .weight(1f)
                    .nestedScroll(nestedScrollConnection)
                    .padding(horizontal = 12.dp),
                contentPadding = PaddingValues(bottom = 88.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                item { Spacer(Modifier.height(4.dp)) }
                items(entries.size, key = { it }) { index ->
                    MagiskLogCard(entries[index])
                }
                item { Spacer(Modifier.height(4.dp)) }
            }
        }

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp, Alignment.End)
        ) {
            TextButton(
                text = stringResource(CoreR.string.menuSaveLog),
                onClick = onSave
            )
            TextButton(
                text = stringResource(CoreR.string.menuClearLog),
                onClick = onClear
            )
        }
    }
}

@Composable
private fun MagiskLogCard(entry: MagiskLogEntry) {
    var expanded by remember { mutableStateOf(false) }

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { expanded = !expanded }
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            if (entry.isParsed) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(6.dp),
                        modifier = Modifier.weight(1f)
                    ) {
                        LogLevelBadge(entry.level)
                        Text(
                            text = entry.tag,
                            style = MiuixTheme.textStyles.body1,
                            fontWeight = FontWeight.Medium,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis,
                        )
                    }
                    Spacer(Modifier.width(8.dp))
                    Text(
                        text = entry.timestamp,
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace,
                        color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                        maxLines = 1,
                    )
                }
                Spacer(Modifier.height(4.dp))
            }

            Text(
                text = entry.message,
                fontFamily = FontFamily.Monospace,
                fontSize = 12.sp,
                lineHeight = 16.sp,
                color = MiuixTheme.colorScheme.onSurface,
                maxLines = if (expanded) Int.MAX_VALUE else 3,
                overflow = TextOverflow.Ellipsis,
            )
        }
    }
}

@Composable
private fun LogLevelBadge(level: Char) {
    val (bg, fg) = when (level) {
        'V' -> Color(0xFF9E9E9E) to Color.White
        'D' -> Color(0xFF2196F3) to Color.White
        'I' -> Color(0xFF4CAF50) to Color.White
        'W' -> Color(0xFFFFC107) to Color.Black
        'E' -> Color(0xFFF44336) to Color.White
        'F' -> Color(0xFF9C27B0) to Color.White
        else -> Color(0xFF757575) to Color.White
    }
    Box(
        modifier = Modifier
            .clip(RoundedCornerShape(4.dp))
            .background(bg)
            .padding(horizontal = 5.dp, vertical = 1.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = level.toString(),
            fontSize = 10.sp,
            fontWeight = FontWeight.Bold,
            fontFamily = FontFamily.Monospace,
            color = fg,
        )
    }
}
