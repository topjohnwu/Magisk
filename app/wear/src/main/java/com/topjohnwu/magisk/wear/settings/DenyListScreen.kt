package com.topjohnwu.magisk.wear.settings

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.ui.deny.DenyAppState
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.wear.component.AppIcon
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.core.R as CoreR

/**
 * The apps on the DenyList. Unlike the phone app, an app is switched as a whole with
 * [DenyListViewModel.toggleAll]; its processes can't be picked one by one here.
 */
@Composable
fun DenyListScreen(viewModel: DenyListViewModel) {
    val loading by viewModel.loading.collectAsStateWithLifecycle()
    val apps by viewModel.filteredApps.collectAsStateWithLifecycle()
    val showSystem by viewModel.showSystem.collectAsStateWithLifecycle()

    // Kept across the loading screen, so the list doesn't scroll back when it ends
    val state = rememberTransformingLazyColumnState()

    if (loading) {
        LoadingScreen()
        return
    }

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.denylist)) }
            }
            items(
                items = apps,
                key = { it.info.packageName },
            ) { app ->
                DenyAppRow(app = app, onToggle = { viewModel.toggleAll(app) })
            }
            item(key = "show_system") {
                SwitchButton(
                    checked = showSystem,
                    onCheckedChange = viewModel::setShowSystem,
                    label = { Text(stringResource(CoreR.string.show_system_app)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
private fun DenyAppRow(app: DenyAppState, onToggle: () -> Unit) {
    SwitchButton(
        // On when any of its processes is denied; switching it then removes them all,
        // like tapping the partly checked box in the phone app
        checked = app.isChecked,
        onCheckedChange = { onToggle() },
        modifier = Modifier.fillMaxWidth(),
        label = {
            AppIcon(packageName = app.info.packageName, size = 28.dp)
            Column(
                modifier = Modifier.weight(1f).padding(start = 8.dp),
                verticalArrangement = Arrangement.Center,
            ) {
                Text(
                    text = app.info.label,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                Text(
                    text = app.info.packageName,
                    style = MaterialTheme.typography.labelSmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
            }
        },
    )
}
