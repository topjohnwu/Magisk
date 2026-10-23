package com.topjohnwu.magisk.ui.superuser

import androidx.compose.foundation.Image
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
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Cancel
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ui.component.AppIcon
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.SettingsSwitch
import com.topjohnwu.magisk.ui.component.rememberBiometricAuth
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuperuserDetailScreen(
    uid: Int,
    viewModel: SuperuserViewModel,
    onBack: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    // After a revoke, the policy is removed from the state while this screen is still
    // animating out, so keep showing the last known content instead of a blank page.
    val lastItems = remember(uid) { LastItems() }
    val currentItems = uiState.policies.filter { it.uid == uid }
    val items = currentItems
        .also { if (it.isNotEmpty()) lastItems.value = it }
        .ifEmpty { lastItems.value }
    val item = items.firstOrNull()

    // If the policy disappears while this screen is still shown (e.g. the app was uninstalled),
    // leave instead of showing stale content. After a revoke, we have already navigated away.
    val navigator = LocalNavigator.current
    val isGone = !uiState.loading && currentItems.isEmpty()
    LaunchedEffect(isGone) {
        if (isGone && navigator.current() == Route.SuperuserDetail(uid)) onBack()
    }
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val scrollState = rememberScrollState()
    val scope = rememberCoroutineScope()
    val requireAuth = rememberBiometricAuth()
    val revokeDialog = rememberConfirmDialog()
    val revokeTitle = stringResource(CoreR.string.su_revoke_title)
    val revokeMsg = item?.let { stringResource(CoreR.string.su_revoke_msg, it.appName) } ?: ""

    LaunchedEffect(Unit) {
        viewModel.refreshSuRestrict()
        if (uiState.policies.isEmpty()) {
            viewModel.startLoading()
        }
    }

    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.settings)) },
                navigationIcon = {
                    IconButton(
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = stringResource(CoreR.string.back),
                        )
                    }
                },
                scrollBehavior = scrollBehavior
            )
        }
    ) { padding ->
        if (uiState.loading) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                CircularProgressIndicator()
            }
            return@Scaffold
        }

        if (item == null) return@Scaffold

        Column(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .verticalScrollbar(scrollState, contentPadding = PaddingValues(vertical = 12.dp))
                .verticalScroll(scrollState)
                .wrapContentWidth(Alignment.CenterHorizontally)
                .widthIn(max = 720.dp)
                .padding(horizontal = 12.dp, vertical = 12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(20.dp),
                colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
            ) {
                Row(
                    modifier = Modifier.padding(18.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    AppIcon(
                        packageName = item.packageName,
                        contentDescription = item.appName,
                        modifier = Modifier.size(48.dp)
                    )
                    Spacer(Modifier.width(16.dp))
                    Column {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Text(
                                text = item.title,
                                style = MaterialTheme.typography.titleLarge,
                                modifier = Modifier.weight(1f, fill = false),
                            )
                            if (item.isSharedUid) {
                                Spacer(Modifier.width(8.dp))
                                SharedUidBadge()
                            }
                        }
                        Spacer(Modifier.height(2.dp))
                        Text(
                            text = item.packageName,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Text(
                            text = "UID: ${item.uid}",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(20.dp),
                colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
            ) {
                if (uiState.suRestrict || item.isRestricted) {
                    SettingsSwitch(
                        title = stringResource(CoreR.string.settings_su_restrict_title),
                        checked = item.isRestricted,
                        onCheckedChange = {
                            requireAuth {
                                viewModel.toggleRestrict(item)
                            }
                        }
                    )
                    HorizontalDivider(modifier = Modifier.padding(horizontal = 16.dp))
                }
                SettingsSwitch(
                    title = stringResource(CoreR.string.superuser_toggle_notification),
                    checked = item.notification,
                    onCheckedChange = { viewModel.updateNotify(item) }
                )
                HorizontalDivider(modifier = Modifier.padding(horizontal = 16.dp))
                SettingsSwitch(
                    title = stringResource(CoreR.string.logs),
                    checked = item.logging,
                    onCheckedChange = { viewModel.updateLogging(item) }
                )
            }

            RevokeButton(
                onClick = {
                    if (Config.suAuth) {
                        requireAuth { viewModel.performDelete(item, onBack) }
                    } else {
                        scope.launch {
                            val result = revokeDialog.awaitConfirm(
                                title = revokeTitle,
                                content = revokeMsg,
                            )
                            if (result == ConfirmResult.Confirmed) {
                                viewModel.performDelete(item, onBack)
                            }
                        }
                    }
                }
            )
        }
    }
}

@Composable
private fun RevokeButton(
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Button(
        onClick = onClick,
        colors = ButtonDefaults.filledTonalButtonColors(
            containerColor = MaterialTheme.colorScheme.errorContainer,
            contentColor = MaterialTheme.colorScheme.onErrorContainer
        ),
        shape = ButtonDefaults.shape,
        contentPadding = ButtonDefaults.ButtonWithIconContentPadding,
        modifier = modifier.fillMaxWidth()
    ) {
        Icon(
            imageVector = Icons.Default.Cancel,
            contentDescription = null,
            modifier = Modifier.size(18.dp)
        )
        Spacer(Modifier.width(8.dp))
        Text(
            text = stringResource(CoreR.string.superuser_toggle_revoke),
            style = MaterialTheme.typography.labelLarge,
        )
    }
}

// Plain holder: updating it must not trigger recomposition
private class LastItems {
    var value: List<PolicyItem> = emptyList()
}
