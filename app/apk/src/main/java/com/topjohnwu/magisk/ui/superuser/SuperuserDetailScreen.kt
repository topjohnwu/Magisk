package com.topjohnwu.magisk.ui.superuser

import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
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
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.util.rememberDrawablePainter
import kotlinx.coroutines.launch
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Switch
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Back
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun SuperuserDetailScreen(
    uid: Int,
    viewModel: SuperuserViewModel,
    onBack: () -> Unit,
) {
    val uiState by viewModel.uiState.collectAsState()
    val items = uiState.policies.filter { it.policy.uid == uid }
    val item = items.firstOrNull()
    val scrollBehavior = MiuixScrollBehavior()
    val scope = rememberCoroutineScope()
    val revokeDialog = rememberConfirmDialog()
    val revokeTitle = stringResource(CoreR.string.su_revoke_title)
    val revokeMsg = item?.let { stringResource(CoreR.string.su_revoke_msg, it.appName) } ?: ""

    LaunchedEffect(Unit) { viewModel.refreshSuRestrict() }

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.superuser_setting),
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = MiuixIcons.Back,
                            contentDescription = stringResource(CoreR.string.back),
                        )
                    }
                },
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        if (item == null) return@Scaffold

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .padding(horizontal = 12.dp),
            contentPadding = PaddingValues(bottom = 88.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item {
                Spacer(Modifier.height(4.dp))
            }

            item {
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
                            Text(
                                text = item.title,
                                style = MiuixTheme.textStyles.headline2,
                            )
                            Text(
                                text = item.packageName,
                                style = MiuixTheme.textStyles.body2,
                                color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                            )
                            Text(
                                text = "UID: ${item.policy.uid}",
                                style = MiuixTheme.textStyles.body2,
                                color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                            )
                        }
                    }
                }
            }

            item {
                Card(modifier = Modifier.fillMaxWidth()) {
                    Column {
                        if (uiState.suRestrict || item.isRestricted) {
                            SwitchRow(
                                title = stringResource(CoreR.string.settings_su_restrict_title),
                                summary = stringResource(
                                    if (item.isRestricted) CoreR.string.enabled else CoreR.string.disabled
                                ),
                                checked = item.isRestricted,
                                onCheckedChange = { viewModel.toggleRestrict(item) }
                            )
                        }
                        SwitchRow(
                            title = stringResource(CoreR.string.superuser_toggle_notification),
                            summary = stringResource(
                                if (item.notification) CoreR.string.enabled else CoreR.string.disabled
                            ),
                            checked = item.notification,
                            onCheckedChange = { viewModel.updateNotify(item) }
                        )
                        SwitchRow(
                            title = stringResource(CoreR.string.logs),
                            summary = stringResource(
                                if (item.logging) CoreR.string.enabled else CoreR.string.disabled
                            ),
                            checked = item.logging,
                            onCheckedChange = { viewModel.updateLogging(item) }
                        )
                    }
                }
            }

            item {
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable {
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
                ) {
                    RevokeRow()
                }
            }
        }
    }
}

@Composable
private fun SwitchRow(
    title: String,
    summary: String,
    checked: Boolean,
    onCheckedChange: () -> Unit,
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = title,
                style = MiuixTheme.textStyles.body1,
            )
            Text(
                text = summary,
                style = MiuixTheme.textStyles.body2,
                color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
            )
        }
        Spacer(Modifier.width(16.dp))
        Switch(
            checked = checked,
            onCheckedChange = { onCheckedChange() }
        )
    }
}

@Composable
private fun RevokeRow() {
    Text(
        text = stringResource(CoreR.string.superuser_toggle_revoke),
        style = MiuixTheme.textStyles.body1,
        color = MiuixTheme.colorScheme.error,
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 16.dp)
    )
}
