package com.topjohnwu.magisk.ui.component

import androidx.annotation.StringRes
import androidx.compose.foundation.layout.Box
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Sort
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Tune
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.R as CoreR

enum class AppSortBy(@StringRes val labelRes: Int) {
    NAME(CoreR.string.sort_by_name),
    PACKAGE_NAME(CoreR.string.sort_by_package_name),
    INSTALL_TIME(CoreR.string.sort_by_install_time),
    UPDATE_TIME(CoreR.string.sort_by_update_time);
}

@Composable
fun AppSortFilterActions(
    sortBy: AppSortBy,
    sortReverse: Boolean,
    onSortByChanged: (AppSortBy) -> Unit,
    onToggleSortReverse: () -> Unit,
    showSystem: Boolean,
    showOS: Boolean,
    onShowSystemChanged: (Boolean) -> Unit,
    onShowOSChanged: (Boolean) -> Unit,
    modifier: Modifier = Modifier,
) {
    var showSortMenu by remember { mutableStateOf(false) }
    var showFilterMenu by remember { mutableStateOf(false) }

    Box(modifier) {
        IconButton(onClick = { showSortMenu = true }) {
            Icon(
                imageVector = Icons.AutoMirrored.Filled.Sort,
                contentDescription = stringResource(CoreR.string.menu_sort),
            )
        }
        DropdownMenu(
            expanded = showSortMenu,
            onDismissRequest = { showSortMenu = false }
        ) {
            AppSortBy.entries.forEach { sort ->
                DropdownMenuItem(
                    text = { Text(stringResource(sort.labelRes)) },
                    trailingIcon = if (sortBy == sort) {
                        { Icon(Icons.Default.Check, contentDescription = null) }
                    } else null,
                    onClick = {
                        onSortByChanged(sort)
                        showSortMenu = false
                    }
                )
            }
            DropdownMenuItem(
                text = { Text(stringResource(CoreR.string.sort_reverse)) },
                trailingIcon = if (sortReverse) {
                    { Icon(Icons.Default.Check, contentDescription = null) }
                } else null,
                onClick = {
                    onToggleSortReverse()
                    showSortMenu = false
                }
            )
        }
    }

    Box {
        IconButton(onClick = { showFilterMenu = true }) {
            Icon(
                imageVector = Icons.Default.Tune,
                contentDescription = stringResource(CoreR.string.hide_filter_hint),
            )
        }
        DropdownMenu(
            expanded = showFilterMenu,
            onDismissRequest = { showFilterMenu = false },
            offset = DpOffset(x = (-8).dp, y = 0.dp),
        ) {
            DropdownMenuItem(
                text = { Text(stringResource(CoreR.string.show_system_app)) },
                trailingIcon = if (showSystem) {
                    { Icon(Icons.Default.Check, contentDescription = null) }
                } else null,
                onClick = {
                    onShowSystemChanged(!showSystem)
                    showFilterMenu = false
                }
            )
            DropdownMenuItem(
                text = { Text(stringResource(CoreR.string.show_os_app)) },
                trailingIcon = if (showOS) {
                    { Icon(Icons.Default.Check, contentDescription = null) }
                } else null,
                onClick = {
                    if (!showOS && !showSystem) {
                        onShowSystemChanged(true)
                    }
                    onShowOSChanged(!showOS)
                    showFilterMenu = false
                }
            )
        }
    }
}
