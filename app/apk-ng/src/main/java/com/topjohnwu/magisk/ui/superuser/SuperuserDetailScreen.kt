package com.topjohnwu.magisk.ui.superuser

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Cancel
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.SettingsSwitch
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuperuserDetailScreen(
    uid: Int,
    viewModel: SuperuserViewModel,
    onBack: () -> Unit,
) {
    val uiState by viewModel.uiState.collectAsState()
    val items = uiState.policies.filter { it.policy.uid == uid }
    val item = items.firstOrNull()
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val scope = rememberCoroutineScope()
    val revokeDialog = rememberConfirmDialog()
    val revokeTitle = stringResource(CoreR.string.su_revoke_title)
    val revokeMsg = item?.let { stringResource(CoreR.string.su_revoke_msg, it.appName) } ?: ""

    LaunchedEffect(Unit) { viewModel.refreshSuRestrict() }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.superuser_setting)) },
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
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
        if (item == null) return@Scaffold

        Column(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .verticalScroll(rememberScrollState())
                .padding(padding)
                .padding(horizontal = 12.dp)
                .padding(bottom = 88.dp)
        ) {
            Card(modifier = Modifier.fillMaxWidth()) {
                Row(
                    modifier = Modifier.padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Image(
                        painter = rememberDrawablePainter(item.icon),
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
                        Text(
                            text = item.packageName,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Text(
                            text = "UID: ${item.policy.uid}",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            if (uiState.suRestrict || item.isRestricted) {
                SettingsSwitch(
                    title = stringResource(CoreR.string.settings_su_restrict_title),
                    checked = item.isRestricted,
                    onCheckedChange = { viewModel.toggleRestrict(item) }
                )
            }
            SettingsSwitch(
                title = stringResource(CoreR.string.superuser_toggle_notification),
                checked = item.notification,
                onCheckedChange = { viewModel.updateNotify(item) }
            )
            SettingsSwitch(
                title = stringResource(CoreR.string.logs),
                checked = item.logging,
                onCheckedChange = { viewModel.updateLogging(item) }
            )

            RevokeButton {
                if (viewModel.requiresAuth) {
                    viewModel.authenticate { viewModel.performDelete(item, onBack) }
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
        }
    }
}

@Composable
private fun RevokeButton(
    onClick: () -> Unit,
) {
    Button(
        onClick = onClick,
        colors = ButtonDefaults.buttonColors(
            containerColor = MaterialTheme.colorScheme.errorContainer,
            contentColor = MaterialTheme.colorScheme.onErrorContainer
        ),
        shape = RoundedCornerShape(16.dp),
        modifier = Modifier.fillMaxWidth()
    ) {
        Icon(
            imageVector = Icons.Default.Cancel,
            contentDescription = null,
            modifier = Modifier.size(18.dp)
        )
        Spacer(Modifier.width(8.dp))
        Text(
            text = stringResource(CoreR.string.superuser_toggle_revoke),
            fontSize = 16.sp
        )
    }
}
