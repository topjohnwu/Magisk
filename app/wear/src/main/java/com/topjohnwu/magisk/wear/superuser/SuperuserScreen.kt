package com.topjohnwu.magisk.wear.superuser

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.BugReport
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.IconButton
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SplitSwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.ui.component.rememberBiometricAuth
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.superuser.PolicyItem
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.wear.component.AppIcon
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.nav.CollectNavEvents
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import com.topjohnwu.magisk.core.R as CoreR

/**
 * The apps with a Superuser policy. [viewModel] is shared with [SuperuserDetailScreen], so
 * changes made there show up here right away.
 */
@Composable
fun SuperuserScreen(
    viewModel: SuperuserViewModel,
    navigator: WearNavigator,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    // Asks for authentication first when the Superuser setting requires it
    val requireAuth = rememberBiometricAuth()

    // Reload whenever the screen is shown again, like the phone app does when its tab is selected
    LaunchedEffect(viewModel) {
        viewModel.startLoading()
    }
    CollectNavEvents(viewModel, navigator)

    // Kept across the loading screen, so the list doesn't scroll back when it ends
    val state = rememberTransformingLazyColumnState()

    if (uiState.loading) {
        LoadingScreen()
        return
    }

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(stringResource(CoreR.string.superuser))
                        IconButton(onClick = { navigator.navigate(Route.SuperuserGrant) }) {
                            Icon(
                                Icons.Filled.Add,
                                contentDescription = stringResource(CoreR.string.superuser_grant),
                            )
                        }
                    }
                }
            }
            if (uiState.policies.isEmpty()) {
                item {
                    Text(
                        text = stringResource(CoreR.string.superuser_policy_none),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
                    )
                }
            } else {
                items(
                    items = uiState.policies,
                    key = { "${it.uid}_${it.packageName}" },
                ) { item ->
                    PolicyRow(
                        item = item,
                        onToggle = { requireAuth { viewModel.togglePolicy(item) } },
                        onDetail = { navigator.navigate(Route.SuperuserDetail(item.uid)) },
                    )
                }
            }
            item {
                FilledTonalButton(
                    onClick = { navigator.navigate(WearRoute.Logs) },
                    icon = { Icon(Icons.Filled.BugReport, contentDescription = null) },
                    label = { Text(stringResource(CoreR.string.wear_su_logs)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
private fun PolicyRow(
    item: PolicyItem,
    onToggle: () -> Unit,
    onDetail: () -> Unit,
) {
    SplitSwitchButton(
        checked = item.isEnabled,
        // The view model flips the latest state, so quick repeated taps never get lost
        onCheckedChange = { onToggle() },
        toggleContentDescription = item.appName,
        onContainerClick = onDetail,
        modifier = Modifier.fillMaxWidth(),
        label = {
            AppIcon(packageName = item.packageName, size = 28.dp)
            Column(
                modifier = Modifier.weight(1f).padding(start = 8.dp),
                verticalArrangement = Arrangement.Center,
            ) {
                Text(
                    text = item.title,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                Text(
                    text = stringResource(rootLabel(item)),
                    style = MaterialTheme.typography.labelSmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
            }
        },
    )
}
