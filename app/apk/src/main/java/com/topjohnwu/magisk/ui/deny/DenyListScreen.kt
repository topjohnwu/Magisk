package com.topjohnwu.magisk.ui.deny

import androidx.compose.animation.AnimatedVisibility
import android.graphics.Bitmap
import android.graphics.drawable.Drawable
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
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
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.unit.dp
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Checkbox
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.LinearProgressIndicator
import top.yukonga.miuix.kmp.basic.Switch
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun DenyListScreen(viewModel: DenyListViewModel) {
    val loading by viewModel.loading.collectAsState()
    val apps by viewModel.filteredApps.collectAsState()
    val query by viewModel.query.collectAsState()
    val showSystem by viewModel.showSystem.collectAsState()
    val showOS by viewModel.showOS.collectAsState()

    Column(modifier = Modifier.fillMaxSize()) {
        // Search input
        SearchInput(
            query = query,
            onQueryChange = viewModel::setQuery,
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp, vertical = 4.dp)
        )

        // Filter chips
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            FilterChip(
                label = stringResource(CoreR.string.show_system_app),
                checked = showSystem,
                onCheckedChange = viewModel::setShowSystem
            )
            FilterChip(
                label = stringResource(CoreR.string.show_os_app),
                checked = showOS,
                enabled = showSystem,
                onCheckedChange = viewModel::setShowOS
            )
        }

        Spacer(Modifier.height(8.dp))

        if (loading) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = stringResource(CoreR.string.loading),
                        style = MiuixTheme.textStyles.headline2
                    )
                    Spacer(Modifier.height(16.dp))
                    CircularProgressIndicator()
                }
            }
        } else {
            LazyColumn(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 12.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                items(
                    items = apps,
                    key = { it.info.packageName }
                ) { app ->
                    DenyAppCard(app)
                }
                item { Spacer(Modifier.height(8.dp)) }
            }
        }
    }
}

@Composable
private fun SearchInput(query: String, onQueryChange: (String) -> Unit, modifier: Modifier = Modifier) {
    top.yukonga.miuix.kmp.basic.TextField(
        value = query,
        onValueChange = onQueryChange,
        modifier = modifier,
        label = stringResource(CoreR.string.hide_filter_hint)
    )
}

@Composable
private fun FilterChip(
    label: String,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit,
    enabled: Boolean = true
) {
    Row(
        modifier = Modifier
            .clickable(enabled = enabled) { onCheckedChange(!checked) }
            .padding(vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        Checkbox(
            checked = checked,
            onCheckedChange = if (enabled) onCheckedChange else null,
            enabled = enabled
        )
        Text(
            text = label,
            style = MiuixTheme.textStyles.body2,
            color = if (enabled) MiuixTheme.colorScheme.onSurface
                else MiuixTheme.colorScheme.disabledOnSecondaryVariant
        )
    }
}

@Composable
private fun DenyAppCard(app: DenyAppState) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column {
            // Progress bar showing percentage of checked processes
            if (app.checkedPercent > 0f) {
                LinearProgressIndicator(
                    progress = app.checkedPercent,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            // App row
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { app.isExpanded = !app.isExpanded }
                    .padding(12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Image(
                    painter = rememberDrawablePainter(app.info.iconImage),
                    contentDescription = app.info.label,
                    modifier = Modifier.size(40.dp)
                )
                Spacer(Modifier.width(12.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = app.info.label,
                        style = MiuixTheme.textStyles.body1,
                    )
                    Text(
                        text = app.info.packageName,
                        style = MiuixTheme.textStyles.body2,
                        color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                    )
                }
                Checkbox(
                    checked = app.isChecked,
                    onCheckedChange = { app.toggleAll() }
                )
            }

            // Expanded process list
            AnimatedVisibility(visible = app.isExpanded) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(start = 52.dp)
                ) {
                    app.processes.forEach { proc ->
                        ProcessRow(proc)
                    }
                }
            }
        }
    }
}

@Composable
private fun ProcessRow(proc: DenyProcessState) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { proc.toggle() }
            .padding(horizontal = 12.dp, vertical = 6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = proc.displayName,
            style = MiuixTheme.textStyles.body2,
            color = if (proc.isEnabled) MiuixTheme.colorScheme.onSurface
                else MiuixTheme.colorScheme.onSurfaceVariantSummary,
            modifier = Modifier.weight(1f)
        )
        Switch(
            checked = proc.isEnabled,
            onCheckedChange = { proc.toggle() }
        )
    }
}

@Composable
private fun rememberDrawablePainter(drawable: Drawable): Painter {
    return remember(drawable) {
        val w = drawable.intrinsicWidth.coerceAtLeast(1)
        val h = drawable.intrinsicHeight.coerceAtLeast(1)
        val bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
        val canvas = android.graphics.Canvas(bitmap)
        drawable.setBounds(0, 0, w, h)
        drawable.draw(canvas)
        BitmapPainter(bitmap.asImageBitmap())
    }
}
