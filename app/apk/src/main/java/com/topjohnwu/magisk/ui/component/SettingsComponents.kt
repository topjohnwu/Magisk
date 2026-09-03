package com.topjohnwu.magisk.ui.component

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material3.Icon
import androidx.compose.material3.ListItem
import androidx.compose.material3.ListItemDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp

@Composable
fun SettingsArrow(
    title: String,
    modifier: Modifier = Modifier,
    summary: String? = null,
    leadingContent: @Composable (() -> Unit)? = null,
    onClick: () -> Unit
) {
    ListItem(
        headlineContent = { Text(title, style = MaterialTheme.typography.bodyLarge) },
        supportingContent = summary?.let { { Text(it, style = MaterialTheme.typography.bodyMedium) } },
        leadingContent = leadingContent,
        trailingContent = { Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null) },
        colors = ListItemDefaults.colors(containerColor = Color.Transparent),
        modifier = modifier.clickable(onClick = onClick)
    )
}

@Composable
fun SettingsSwitch(
    title: String,
    checked: Boolean,
    modifier: Modifier = Modifier,
    summary: String? = null,
    enabled: Boolean = true,
    onCheckedChange: (Boolean) -> Unit
) {
    ListItem(
        headlineContent = { Text(title, style = MaterialTheme.typography.bodyLarge) },
        supportingContent = summary?.takeIf { it.isNotEmpty() }?.let { { Text(it, style = MaterialTheme.typography.bodyMedium) } },
        trailingContent = { Switch(checked = checked, onCheckedChange = onCheckedChange, enabled = enabled) },
        colors = ListItemDefaults.colors(containerColor = Color.Transparent),
        modifier = modifier.clickable(enabled = enabled, onClick = { onCheckedChange(!checked) })
    )
}

@Composable
fun SettingsDropdown(
    title: String,
    items: List<String>,
    selectedIndex: Int,
    modifier: Modifier = Modifier,
    summary: String? = null,
    enabled: Boolean = true,
    onSelectedIndexChange: (Int) -> Unit
) {
    var showDialog by remember { mutableStateOf(false) }

    ListItem(
        headlineContent = { Text(title, style = MaterialTheme.typography.bodyLarge) },
        supportingContent = {
            val currentSummary = summary ?: items.getOrNull(selectedIndex) ?: ""
            if (currentSummary.isNotEmpty()) Text(currentSummary, style = MaterialTheme.typography.bodyMedium)
        },
        colors = ListItemDefaults.colors(containerColor = Color.Transparent),
        modifier = modifier.clickable(enabled = enabled, onClick = { showDialog = true })
    )

    if (showDialog) {
        MagiskDialog(
            onDismissRequest = { showDialog = false },
            title = title,
            dismissText = stringResource(android.R.string.cancel),
            onDismiss = { showDialog = false }
        ) {
            val scrollState = rememberScrollState()
            Column(
                modifier = Modifier
                    .verticalScroll(scrollState)
                    .selectableGroup()
            ) {
                items.forEachIndexed { index, item ->
                    val isSelected = index == selectedIndex
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .selectable(
                                selected = isSelected,
                                onClick = {
                                    onSelectedIndexChange(index)
                                    showDialog = false
                                },
                                role = Role.RadioButton
                            )
                            .padding(vertical = 12.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = isSelected,
                            onClick = null
                        )
                        Spacer(Modifier.width(16.dp))
                        Text(
                            text = item,
                            style = MaterialTheme.typography.bodyLarge,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun SmallTitle(
    text: String,
    modifier: Modifier = Modifier
) {
    Text(
        text = text,
        style = MaterialTheme.typography.labelLarge,
        fontWeight = FontWeight.SemiBold,
        color = MaterialTheme.colorScheme.primary,
        modifier = modifier.padding(start = 16.dp, top = 8.dp, bottom = 8.dp)
    )
}
