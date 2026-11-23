package com.topjohnwu.magisk.wear.log

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.selected
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.TransformingLazyColumnScope
import androidx.wear.compose.foundation.lazy.TransformingLazyColumnState
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.ButtonDefaults
import androidx.wear.compose.material3.Card
import androidx.wear.compose.material3.CompactButton
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.dateFormat
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.log.MagiskLogEntry
import com.topjohnwu.magisk.wear.component.LoadingScreen
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId
import java.time.format.DateTimeFormatter
import java.time.format.FormatStyle
import com.topjohnwu.magisk.core.R as CoreR

private val shortTimeFormat: DateTimeFormatter by lazy {
    DateTimeFormatter.ofLocalizedTime(FormatStyle.SHORT).withZone(ZoneId.systemDefault())
}

/** A row of the Superuser log: a day header, or one request. */
private sealed interface SuLogRow {
    data class Day(val date: LocalDate) : SuLogRow
    data class Entry(val log: SuLog) : SuLogRow
}

/** Puts a header before the first request of each day. [logs] are newest first. */
private fun withDayHeaders(logs: List<SuLog>): List<SuLogRow> {
    val zone = ZoneId.systemDefault()
    val rows = ArrayList<SuLogRow>(logs.size + 8)
    var lastDate: LocalDate? = null
    for (log in logs) {
        val date = Instant.ofEpochMilli(log.time).atZone(zone).toLocalDate()
        if (date != lastDate) {
            rows.add(SuLogRow.Day(date))
            lastDate = date
        }
        rows.add(SuLogRow.Entry(log))
    }
    return rows
}

/**
 * The Superuser and Magisk logs. Without root there is no Superuser log, so only the Magisk
 * log is shown, like the phone app does.
 */
@Composable
fun LogScreen(viewModel: LogViewModel) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val hasRoot = Info.env.isActive
    var magiskTab by rememberSaveable { mutableStateOf(false) }
    val showMagisk = !hasRoot || magiskTab
    var showClear by rememberSaveable { mutableStateOf(false) }

    LaunchedEffect(viewModel) {
        viewModel.startLoading()
    }

    // One state per log, so switching between them keeps each one's position. Kept across the
    // loading screen, so the list doesn't scroll back when it ends.
    val suState = rememberTransformingLazyColumnState()
    val magiskState = rememberTransformingLazyColumnState()

    if (uiState.loading) {
        LoadingScreen()
        return
    }

    val suRows = remember(uiState.suLogs) { withDayHeaders(uiState.suLogs) }
    // Newest first, like the Superuser log, so the latest lines are right below the switch
    val magiskEntries = remember(uiState.magiskLogEntries) { uiState.magiskLogEntries.asReversed() }
    val isEmpty = if (showMagisk) magiskEntries.isEmpty() else suRows.isEmpty()
    val state: TransformingLazyColumnState = if (showMagisk) magiskState else suState

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                if (hasRoot) {
                    LogSwitch(magiskTab = magiskTab, onSelect = { magiskTab = it })
                } else {
                    ListHeader { Text(stringResource(CoreR.string.logs)) }
                }
            }
            if (isEmpty) {
                item {
                    Text(
                        text = stringResource(
                            if (showMagisk) CoreR.string.log_data_magisk_none
                            else CoreR.string.log_data_none
                        ),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.fillMaxWidth().padding(8.dp),
                    )
                }
            } else {
                if (showMagisk) {
                    magiskLogItems(magiskEntries)
                } else {
                    suLogItems(suRows)
                }
                item {
                    FilledTonalButton(
                        onClick = { showClear = true },
                        icon = { Icon(Icons.Filled.Delete, contentDescription = null) },
                        label = { Text(stringResource(CoreR.string.menuClearLog)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
        }
    }

    AlertDialog(
        visible = showClear,
        onDismissRequest = { showClear = false },
        title = { Text(stringResource(CoreR.string.menuClearLog)) },
        text = {
            Text(
                stringResource(
                    CoreR.string.wear_log_clear_msg,
                    stringResource(if (showMagisk) CoreR.string.magisk else CoreR.string.superuser),
                )
            )
        },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                showClear = false
                if (showMagisk) viewModel.clearMagiskLog() else viewModel.clearLog()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { showClear = false })
        },
    )
}

/** Switches between the Superuser log and the Magisk log. */
@Composable
private fun LogSwitch(magiskTab: Boolean, onSelect: (Boolean) -> Unit) {
    Row(
        modifier = Modifier
            .background(MaterialTheme.colorScheme.surfaceContainer, CircleShape)
            .padding(4.dp),
        horizontalArrangement = Arrangement.spacedBy(4.dp),
    ) {
        SwitchSegment(
            label = stringResource(CoreR.string.wear_log_su),
            selected = !magiskTab,
            onClick = { onSelect(false) },
        )
        SwitchSegment(
            label = stringResource(CoreR.string.magisk),
            selected = magiskTab,
            onClick = { onSelect(true) },
        )
    }
}

@Composable
private fun SwitchSegment(label: String, selected: Boolean, onClick: () -> Unit) {
    CompactButton(
        onClick = onClick,
        label = { Text(label) },
        colors = if (selected) ButtonDefaults.buttonColors() else ButtonDefaults.childButtonColors(),
        modifier = Modifier.semantics { this.selected = selected },
    )
}

private fun TransformingLazyColumnScope.suLogItems(rows: List<SuLogRow>) {
    items(
        items = rows,
        key = {
            when (it) {
                is SuLogRow.Day -> "day_${it.date}"
                is SuLogRow.Entry -> "log_${it.log.id}"
            }
        },
    ) { row ->
        when (row) {
            is SuLogRow.Day -> ListHeader { Text(dayLabel(row.date)) }
            is SuLogRow.Entry -> SuLogCard(row.log)
        }
    }
}

@Composable
private fun dayLabel(date: LocalDate): String {
    val today = LocalDate.now(ZoneId.systemDefault())
    return when (date) {
        today -> stringResource(CoreR.string.wear_log_today)
        today.minusDays(1) -> stringResource(CoreR.string.wear_log_yesterday)
        else -> dateFormat.format(date)
    }
}

@Composable
private fun SuLogCard(log: SuLog) {
    val colors = MaterialTheme.colorScheme
    val allowed = log.action >= SuPolicy.ALLOW
    // The command can be long, so show all of it only when asked
    var expanded by rememberSaveable(log.id) { mutableStateOf(false) }
    val result = stringResource(if (allowed) CoreR.string.granted else CoreR.string.denied)

    Card(
        onClick = { expanded = !expanded },
        modifier = Modifier.fillMaxWidth(),
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(
                modifier = Modifier
                    .size(10.dp)
                    .background(if (allowed) colors.primary else colors.error, CircleShape),
            )
            Spacer(Modifier.width(10.dp))
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = log.appName,
                    style = MaterialTheme.typography.titleSmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                Text(
                    text = if (log.command.isEmpty()) result
                    else stringResource(CoreR.string.wear_log_su_result, result, log.command),
                    style = MaterialTheme.typography.labelSmall,
                    color = colors.onSurfaceVariant,
                    maxLines = if (expanded) Int.MAX_VALUE else 1,
                    overflow = TextOverflow.Ellipsis,
                )
            }
            Spacer(Modifier.width(8.dp))
            Text(
                text = log.time.toTime(shortTimeFormat),
                style = MaterialTheme.typography.labelSmall,
                color = colors.onSurfaceVariant,
            )
        }
    }
}

private fun TransformingLazyColumnScope.magiskLogItems(entries: List<MagiskLogEntry>) {
    // One item per entry, so a long log is only laid out as far as it is scrolled. The entries
    // are newest first, so key them by their position in the log, which a reload keeps.
    items(count = entries.size, key = { entries.size - 1 - it }) { index ->
        MagiskLogLine(entries[index])
    }
}

@Composable
private fun MagiskLogLine(entry: MagiskLogEntry) {
    val colors = MaterialTheme.colorScheme
    val mono = MaterialTheme.typography.bodySmall.copy(fontFamily = FontFamily.Monospace)
    // Long messages are cut to a few lines, like on the phone; a tap shows all of it
    var expanded by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { expanded = !expanded }
            .padding(horizontal = 8.dp, vertical = 4.dp),
    ) {
        if (entry.isParsed) {
            Text(
                text = "${entry.level} ${entry.timestamp}",
                style = MaterialTheme.typography.labelSmall.copy(fontFamily = FontFamily.Monospace),
                color = when (entry.level) {
                    'E', 'F' -> colors.error
                    'W' -> colors.tertiary
                    else -> colors.onSurfaceVariant
                },
                maxLines = 1,
            )
        }
        Text(
            text = entry.message,
            style = mono,
            color = colors.onSurface,
            maxLines = if (expanded) Int.MAX_VALUE else 3,
            overflow = TextOverflow.Ellipsis,
        )
    }
}
