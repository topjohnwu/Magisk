package com.topjohnwu.magisk.ui.superuser

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
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
import androidx.compose.ui.draw.alpha
import com.topjohnwu.magisk.ui.util.rememberDrawablePainter
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.model.su.SuPolicy
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Slider
import top.yukonga.miuix.kmp.basic.Switch
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun SuperuserScreen(viewModel: SuperuserViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val scrollBehavior = MiuixScrollBehavior()

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.superuser),
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

        if (uiState.policies.isEmpty()) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = stringResource(CoreR.string.superuser_policy_none),
                    style = MiuixTheme.textStyles.body1,
                    color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                )
            }
            return@Scaffold
        }

        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(horizontal = 12.dp),
            contentPadding = PaddingValues(bottom = 88.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }
            items(uiState.policies, key = { "${it.policy.uid}_${it.packageName}" }) { item ->
                PolicyCard(item = item, viewModel = viewModel)
            }
            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun PolicyCard(item: PolicyItem, viewModel: SuperuserViewModel) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .alpha(if (item.isEnabled) 1f else 0.5f)
    ) {
        Column {
            Row(
                modifier = Modifier
                    .clickable { item.isExpanded = !item.isExpanded }
                    .padding(16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Image(
                    painter = rememberDrawablePainter(item.icon),
                    contentDescription = item.appName,
                    modifier = Modifier.size(40.dp)
                )
                Spacer(Modifier.width(12.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = item.title,
                        style = MiuixTheme.textStyles.body1,
                    )
                    Text(
                        text = item.packageName,
                        style = MiuixTheme.textStyles.body2,
                        color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                    )
                }
                Spacer(Modifier.width(8.dp))
                if (item.showSlider) {
                    PolicySlider(
                        value = item.policyValue,
                        onValueChange = { viewModel.updatePolicy(item, it) }
                    )
                } else {
                    Switch(
                        checked = item.isEnabled,
                        onCheckedChange = { viewModel.togglePolicy(item) }
                    )
                }
            }

            AnimatedVisibility(visible = item.isExpanded) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    horizontalArrangement = Arrangement.SpaceEvenly
                ) {
                    TextButton(
                        text = stringResource(CoreR.string.superuser_toggle_notification),
                        onClick = { viewModel.updateNotify(item) },
                        modifier = Modifier.weight(1f)
                    )
                    TextButton(
                        text = stringResource(CoreR.string.logs),
                        onClick = { viewModel.updateLogging(item) },
                        modifier = Modifier.weight(1f)
                    )
                    TextButton(
                        text = stringResource(CoreR.string.superuser_toggle_revoke),
                        onClick = { viewModel.deletePressed(item) },
                        modifier = Modifier.weight(1f)
                    )
                }
            }
        }
    }
}

@Composable
private fun PolicySlider(value: Int, onValueChange: (Int) -> Unit) {
    val sliderValue = when (value) {
        SuPolicy.DENY -> 0f
        SuPolicy.RESTRICT -> 0.5f
        SuPolicy.ALLOW -> 1f
        else -> 0f
    }
    val label = when (value) {
        SuPolicy.DENY -> stringResource(CoreR.string.deny)
        SuPolicy.RESTRICT -> stringResource(CoreR.string.restrict)
        SuPolicy.ALLOW -> stringResource(CoreR.string.grant)
        else -> stringResource(CoreR.string.deny)
    }

    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier.width(96.dp)
    ) {
        Text(
            text = label,
            style = MiuixTheme.textStyles.body2,
            color = MiuixTheme.colorScheme.onSurfaceVariantSummary
        )
        Slider(
            value = sliderValue,
            onValueChange = { newVal ->
                val newPolicy = when {
                    newVal < 0.25f -> SuPolicy.DENY
                    newVal < 0.75f -> SuPolicy.RESTRICT
                    else -> SuPolicy.ALLOW
                }
                if (newPolicy != value) onValueChange(newPolicy)
            },
            steps = 1,
            modifier = Modifier.fillMaxWidth()
        )
    }
}

