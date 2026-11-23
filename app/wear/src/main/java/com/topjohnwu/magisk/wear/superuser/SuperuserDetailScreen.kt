package com.topjohnwu.magisk.wear.superuser

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Cancel
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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ui.component.rememberBiometricAuth
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.superuser.PolicyItem
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.wear.component.AppIcon
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.theme.destructiveButtonColors
import com.topjohnwu.magisk.core.R as CoreR

/** Settings of the policy of [uid]. [viewModel] is the one [SuperuserScreen] uses. */
@Composable
fun SuperuserDetailScreen(
    uid: Int,
    viewModel: SuperuserViewModel,
    navigator: WearNavigator,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    // After a revoke, the policy is removed from the state while this screen is still
    // animating out, so keep showing the last known content instead of a blank page.
    val lastItems = remember(uid) { LastItems() }
    val currentItems = uiState.policies.filter { it.uid == uid }
    val policy = currentItems
        .also { if (it.isNotEmpty()) lastItems.value = it }
        .ifEmpty { lastItems.value }
        .firstOrNull()

    // Only leave if this screen is still on top: a revoke confirmed twice, or the policy
    // vanishing right after a revoke, must not pop the screen below it
    val leave: () -> Unit = {
        if (navigator.current() == Route.SuperuserDetail(uid)) navigator.pop()
    }

    // If the policy disappears while this screen is still shown (e.g. the app was uninstalled),
    // leave instead of showing stale content. After a revoke, we have already navigated away.
    val isGone = !uiState.loading && currentItems.isEmpty()
    LaunchedEffect(isGone) {
        if (isGone) leave()
    }

    LaunchedEffect(Unit) {
        viewModel.refreshSuRestrict()
        if (uiState.policies.isEmpty()) {
            viewModel.startLoading()
        }
    }

    var showRevoke by rememberSaveable { mutableStateOf(false) }
    // Asks for authentication first when the Superuser setting requires it
    val requireAuth = rememberBiometricAuth()
    val state = rememberTransformingLazyColumnState()

    if (uiState.loading) {
        LoadingScreen()
        return
    }

    if (policy == null) return

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                AppHeader(policy)
            }
            if (uiState.suRestrict || policy.isRestricted) {
                item {
                    SwitchButton(
                        checked = policy.isRestricted,
                        onCheckedChange = { requireAuth { viewModel.toggleRestrict(policy) } },
                        label = { Text(stringResource(CoreR.string.settings_su_restrict_title)) },
                        modifier = Modifier.fillMaxWidth(),
                    )
                }
            }
            item {
                SwitchButton(
                    checked = policy.notification,
                    onCheckedChange = { viewModel.updateNotify(policy) },
                    label = { Text(stringResource(CoreR.string.superuser_toggle_notification)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            item {
                SwitchButton(
                    checked = policy.logging,
                    onCheckedChange = { viewModel.updateLogging(policy) },
                    label = { Text(stringResource(CoreR.string.logs)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            item {
                Button(
                    onClick = {
                        // Authentication replaces the confirmation, like on the phone
                        if (Config.suAuth) {
                            requireAuth { viewModel.performDelete(policy) { leave() } }
                        } else {
                            showRevoke = true
                        }
                    },
                    colors = destructiveButtonColors(),
                    icon = { Icon(Icons.Filled.Cancel, contentDescription = null) },
                    label = { Text(stringResource(CoreR.string.superuser_toggle_revoke)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }

    AlertDialog(
        visible = showRevoke,
        onDismissRequest = { showRevoke = false },
        title = { Text(stringResource(CoreR.string.su_revoke_title)) },
        text = { Text(stringResource(CoreR.string.su_revoke_msg, policy.appName)) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                showRevoke = false
                viewModel.performDelete(policy) { leave() }
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { showRevoke = false })
        },
    )
}

@Composable
private fun AppHeader(item: PolicyItem) {
    Column(
        modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        AppIcon(
            packageName = item.packageName,
            size = 40.dp,
            contentDescription = item.appName,
        )
        Text(
            text = item.title,
            style = MaterialTheme.typography.titleMedium,
            textAlign = TextAlign.Center,
            modifier = Modifier.padding(top = 4.dp),
        )
        Text(
            text = item.packageName,
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = TextAlign.Center,
        )
        if (item.isSharedUid) {
            Text(
                text = stringResource(CoreR.string.shared_uid),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.primary,
            )
        }
    }
}

// Plain holder: updating it must not trigger recomposition
private class LastItems {
    var value: List<PolicyItem> = emptyList()
}
