package com.topjohnwu.magisk.ui.superuser

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuperuserScreen(
    viewModel: SuperuserViewModel,
    modifier: Modifier = Modifier,
    onRegisterFab: (((() -> Unit)?) -> Unit)? = null,
    contentFocusRequester: FocusRequester? = null,
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    val navigator = LocalNavigator.current

    DisposableEffect(onRegisterFab) {
        onRegisterFab?.invoke { navigator.push(Route.SuperuserGrant) }
        onDispose {
            onRegisterFab?.invoke(null)
        }
    }

    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.superuser)) },
                scrollBehavior = scrollBehavior
            )
        },
        floatingActionButton = {
            if (onRegisterFab == null) {
                FloatingActionButton(
                    onClick = { navigator.push(Route.SuperuserGrant) },
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    contentColor = MaterialTheme.colorScheme.onPrimaryContainer,
                    content = {
                        Icon(
                            imageVector = Icons.Default.Add,
                            contentDescription = stringResource(CoreR.string.superuser_grant),
                            modifier = Modifier.size(28.dp),
                        )
                    },
                )
            }
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
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.spacedBy(12.dp),
                    modifier = Modifier.padding(horizontal = 32.dp)
                ) {
                    Icon(
                        painter = painterResource(CoreR.drawable.ic_superuser),
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f),
                        modifier = Modifier.size(64.dp)
                    )
                    Text(
                        text = stringResource(CoreR.string.superuser_policy_none),
                        style = MaterialTheme.typography.bodyLarge,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        textAlign = TextAlign.Center
                    )
                }
            }
            return@Scaffold
        }

        val listState = rememberLazyListState()
        LazyColumn(
            state = listState,
            modifier = Modifier
                .fillMaxSize()
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .verticalScrollbar(listState, contentPadding = PaddingValues(top = 4.dp, bottom = 80.dp)),
            contentPadding = PaddingValues(start = 12.dp, top = 4.dp, end = 12.dp, bottom = 80.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            item { Spacer(Modifier.height(4.dp)) }
            items(
                items = uiState.policies,
                key = { "${it.policy.uid}_${it.packageName}" },
                contentType = { "PolicyCard" }
            ) { item ->
                val isLast = item == uiState.policies.lastOrNull()
                PolicyCard(
                    item = item,
                    onToggle = { viewModel.togglePolicy(item) },
                    onDetail = { navigator.push(Route.SuperuserDetail(item.policy.uid)) },
                    modifier = if (isLast && contentFocusRequester != null) {
                        Modifier.focusRequester(contentFocusRequester)
                    } else Modifier,
                )
            }
            item { Spacer(Modifier.height(4.dp)) }
        }
    }
}

@Composable
private fun PolicyCard(
    item: PolicyItem,
    onToggle: () -> Unit,
    onDetail: () -> Unit,
    modifier: Modifier = Modifier
) {
    SuperuserAppCard(
        icon = item.icon,
        title = item.title,
        packageName = item.packageName,
        isSharedUid = item.isSharedUid,
        onClick = onDetail,
        contentDescription = item.appName,
        modifier = modifier.alpha(if (item.isEnabled) 1f else 0.5f),
        trailingContent = {
            Switch(
                checked = item.isEnabled,
                onCheckedChange = { onToggle() }
            )
        }
    )
}
