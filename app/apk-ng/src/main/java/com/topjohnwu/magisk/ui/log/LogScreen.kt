package com.topjohnwu.magisk.ui.log

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
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
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Download
import androidx.compose.material3.Badge
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
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
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import com.topjohnwu.magisk.core.ktx.timeDateFormat
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun LogScreen(viewModel: LogViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    var selectedTab by rememberSaveable { mutableIntStateOf(0) }
    val tabTitles = listOf(
        stringResource(CoreR.string.superuser),
        stringResource(CoreR.string.magisk)
    )
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.logs)) },
                actions = {
                    if (selectedTab == 1) {
                        IconButton(onClick = { viewModel.saveMagiskLog() }) {
                            Icon(
                                imageVector = Icons.Default.Download,
                                contentDescription = stringResource(CoreR.string.save_log),
                            )
                        }
                    }
                    IconButton(
                        modifier = Modifier.padding(end = 16.dp),
                        onClick = {
                            if (selectedTab == 0) viewModel.clearLog()
                            else viewModel.clearMagiskLog()
                        }
                    ) {
                        Icon(
                            imageVector = Icons.Default.Delete,
                            contentDescription = stringResource(CoreR.string.clear_log),
                        )
                    }
                },
                scrollBehavior = scrollBehavior
            )
        }
    ) { padding ->
        Column(modifier = Modifier
            .fillMaxSize()
            .padding(padding)
        ) {
            PrimaryTabRow(
                selectedTabIndex = selectedTab,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 8.dp)
            ) {
                tabTitles.forEachIndexed { index, title ->
                    Tab(
                        selected = selectedTab == index,
                        onClick = { selectedTab = index },
                        text = { Text(title) }
                    )
                }
            }

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
                        nestedScrollConnection = scrollBehavior.nestedScrollConnection
                    )
                    1 -> MagiskLogTab(
                        entries = uiState.magiskLogEntries,
                        nestedScrollConnection = scrollBehavior.nestedScrollConnection
                    )
                }
            }
        }
    }
}

@Composable
private fun SuLogTab(logs: List<SuLog>, nestedScrollConnection: NestedScrollConnection) {
    Column(modifier = Modifier.fillMaxSize()) {
        if (logs.isEmpty()) {
            Box(
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.log_data_none),
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = TextAlign.Center,
                )
            }
        } else {
            LazyColumn(
                modifier = Modifier
                    .weight(1f)
                    .nestedScroll(nestedScrollConnection)
                    .padding(horizontal = 12.dp),
                contentPadding = PaddingValues(top = 8.dp, bottom = 88.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                items(logs, key = { it.id }) { log ->
                    SuLogCard(log)
                }
            }
        }
    }
}

@Composable
private fun SuLogCard(log: SuLog) {
    val res = LocalResources.current
    val pm = LocalContext.current.packageManager
    val icon = remember(log.packageName) {
        runCatching {
            pm.getApplicationInfo(log.packageName, 0).loadIcon(pm)
        }.getOrDefault(pm.defaultActivityIcon)
    }
    val allowed = log.action >= 2

    val uidPidText = buildString {
        append("UID: ${log.toUid}  PID: ${log.fromPid}")
        if (log.target != -1) {
            val target = if (log.target == 0) "magiskd" else log.target.toString()
            append("  → $target")
        }
    }

    val details = buildString {
        if (log.context.isNotEmpty()) {
            append(res.getString(CoreR.string.selinux_context, log.context))
        }
        if (log.gids.isNotEmpty()) {
            if (isNotEmpty()) append("\n")
            append(res.getString(CoreR.string.supp_group, log.gids))
        }
        if (log.command.isNotEmpty()) {
            if (isNotEmpty()) append("\n")
            append(log.command)
        }
    }

    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(12.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.Top
            ) {
                Image(
                    painter = rememberDrawablePainter(icon),
                    contentDescription = log.appName,
                    modifier = Modifier.size(36.dp)
                )
                Spacer(Modifier.width(10.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = log.appName,
                        style = MaterialTheme.typography.bodyLarge,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                    )
                    Text(
                        text = uidPidText,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                    )
                }
                Spacer(Modifier.width(8.dp))
                Column(horizontalAlignment = Alignment.End) {
                    Text(
                        text = log.time.toTime(timeDateFormat),
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        maxLines = 1,
                    )
                    Spacer(Modifier.height(4.dp))
                    SuActionBadge(allowed)
                }
            }

            if (details.isNotEmpty()) {
                Spacer(Modifier.height(6.dp))
                Text(
                    text = details,
                    fontFamily = FontFamily.Monospace,
                    fontSize = 12.sp,
                    lineHeight = 16.sp,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
        }
    }
}

@Composable
private fun SuActionBadge(allowed: Boolean) {
    val bg = if (allowed) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.error
    val text = if (allowed) stringResource(CoreR.string.granted) else stringResource(CoreR.string.denied)
    Badge(
        containerColor = bg,
    ) { Text(text = text) }
}

@Composable
private fun MagiskLogTab(
    entries: List<MagiskLogEntry>,
    nestedScrollConnection: NestedScrollConnection
) {
    Column(modifier = Modifier.fillMaxSize()) {
        if (entries.isEmpty()) {
            Box(
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.log_data_magisk_none),
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = TextAlign.Center,
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
                contentPadding = PaddingValues(top = 8.dp, bottom = 88.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                items(entries.size, key = { it }) { index ->
                    MagiskLogCard(entries[index])
                }
            }
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
                            style = MaterialTheme.typography.bodyLarge,
                            fontWeight = FontWeight.Normal,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis,
                        )
                    }
                    Spacer(Modifier.width(8.dp))
                    Text(
                        text = entry.timestamp,
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
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
                color = MaterialTheme.colorScheme.onSurface,
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
