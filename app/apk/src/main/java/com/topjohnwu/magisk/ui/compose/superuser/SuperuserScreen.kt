package com.topjohnwu.magisk.ui.compose.superuser

import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import android.os.Process
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.NavigateNext
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.graphics.drawable.toBitmap
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.R as CoreR
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.Locale

@Composable
fun SuperuserScreen(
    onOpenLogs: () -> Unit = {},
    viewModel: SuperuserComposeViewModel = viewModel(factory = SuperuserComposeViewModel.Factory)
) {
    val state by viewModel.state.collectAsState()
    val activity = LocalContext.current as? UIActivity<*>
    val scope = rememberCoroutineScope()
    val snackbarHostState = remember { SnackbarHostState() }
    var pendingRevoke by remember { mutableStateOf<PolicyUiItem?>(null) }

    LaunchedEffect(state.message) {
        state.message?.let { msg ->
            snackbarHostState.showSnackbar(msg)
            viewModel.consumeMessage()
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        when {
            state.loading -> LoadingState()
            !Info.showSuperUser -> DisabledState()
            else -> Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 24.dp, vertical = 12.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                SuperuserLogsButton(onClick = onOpenLogs)
                if (state.items.isEmpty()) {
                    EmptyState(modifier = Modifier.weight(1f))
                } else {
                    PolicyList(
                        modifier = Modifier.weight(1f),
                        items = state.items,
                        onToggleExpanded = viewModel::toggleExpanded,
                        onToggleEnabled = { item, enabled ->
                            ensureAuthThen(activity, scope) {
                                viewModel.setPolicy(item.uid, if (enabled) SuPolicy.ALLOW else SuPolicy.DENY)
                            }
                        },
                        onSliderFinished = { item, value ->
                            ensureAuthThen(activity, scope) {
                                viewModel.setPolicy(item.uid, sliderValueToPolicy(value))
                            }
                        },
                        onToggleNotify = { item ->
                            ensureAuthThen(activity, scope) { viewModel.toggleNotify(item) }
                        },
                        onToggleLog = { item ->
                            ensureAuthThen(activity, scope) { viewModel.toggleLog(item) }
                        },
                        onRevoke = { item ->
                            if (Config.suAuth) {
                                ensureAuthThen(activity, scope) { viewModel.revoke(item) }
                            } else {
                                pendingRevoke = item
                            }
                        }
                    )
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 110.dp)
        )
    }

    pendingRevoke?.let { item ->
        AlertDialog(
            onDismissRequest = { pendingRevoke = null },
            title = { Text(text = stringResource(id = CoreR.string.su_revoke_title), fontWeight = FontWeight.Black) },
            text = { Text(text = stringResource(id = CoreR.string.su_revoke_msg, item.title)) },
            shape = RoundedCornerShape(28.dp),
            confirmButton = {
                TextButton(onClick = {
                    pendingRevoke = null
                    scope.launch { viewModel.revoke(item) }
                }) {
                    Text(text = stringResource(id = android.R.string.ok), fontWeight = FontWeight.Bold)
                }
            },
            dismissButton = {
                TextButton(onClick = { pendingRevoke = null }) {
                    Text(text = stringResource(id = android.R.string.cancel))
                }
            }
        )
    }
}

private fun ensureAuthThen(activity: UIActivity<*>?, scope: CoroutineScope, block: suspend () -> Unit) {
    if (!Config.suAuth) {
        scope.launch { block() }
        return
    }
    activity?.withAuthentication { ok ->
        if (ok) scope.launch { block() }
    }
}

@Composable
private fun LoadingState() {
    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        CircularProgressIndicator(strokeCap = androidx.compose.ui.graphics.StrokeCap.Round)
    }
}

@Composable
private fun EmptyState(modifier: Modifier = Modifier) {
    Column(
        modifier = modifier.fillMaxSize().padding(32.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Icon(
            imageVector = Icons.Rounded.Security,
            contentDescription = null,
            modifier = Modifier.size(80.dp),
            tint = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f)
        )
        Spacer(Modifier.height(16.dp))
        Text(
            text = stringResource(id = CoreR.string.superuser_policy_none),
            style = MaterialTheme.typography.titleMedium,
            textAlign = TextAlign.Center,
            color = MaterialTheme.colorScheme.outline.copy(alpha = 0.6f)
        )
    }
}

@Composable
private fun DisabledState(modifier: Modifier = Modifier) {
    Box(modifier.fillMaxSize().padding(32.dp), contentAlignment = Alignment.Center) {
        Text(
            text = stringResource(id = CoreR.string.unsupport_nonroot_stub_msg),
            style = MaterialTheme.typography.bodyLarge,
            textAlign = TextAlign.Center
        )
    }
}

@Composable
private fun PolicyList(
    modifier: Modifier = Modifier,
    items: List<PolicyUiItem>,
    onToggleExpanded: (Int, String) -> Unit,
    onToggleEnabled: (PolicyUiItem, Boolean) -> Unit,
    onSliderFinished: (PolicyUiItem, Float) -> Unit,
    onToggleNotify: (PolicyUiItem) -> Unit,
    onToggleLog: (PolicyUiItem) -> Unit,
    onRevoke: (PolicyUiItem) -> Unit,
) {
    LazyColumn(
        modifier = modifier.fillMaxSize(),
        contentPadding = PaddingValues(bottom = 120.dp, top = 4.dp),
        verticalArrangement = Arrangement.spacedBy(24.dp)
    ) {
        items(items, key = { "${it.uid}:${it.packageName}" }) { item ->
            StylishMagiskPolicyCard(
                item = item,
                onToggleExpanded = { onToggleExpanded(item.uid, item.packageName) },
                onToggleEnabled = { onToggleEnabled(item, it) },
                onSliderFinished = { onSliderFinished(item, it) },
                onToggleNotify = { onToggleNotify(item) },
                onToggleLog = { onToggleLog(item) },
                onRevoke = { onRevoke(item) }
            )
        }
    }
}

@Composable
private fun SuperuserLogsButton(onClick: () -> Unit) {
    FilledTonalButton(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .height(52.dp),
        shape = RoundedCornerShape(16.dp)
    ) {
        Icon(Icons.Rounded.HistoryEdu, null, modifier = Modifier.size(20.dp))
        Spacer(Modifier.width(8.dp))
        Text(stringResource(id = CoreR.string.superuser_logs), fontWeight = FontWeight.Bold)
    }
}

@Composable
private fun StylishMagiskPolicyCard(
    item: PolicyUiItem,
    onToggleExpanded: () -> Unit,
    onToggleEnabled: (Boolean) -> Unit,
    onSliderFinished: (Float) -> Unit,
    onToggleNotify: () -> Unit,
    onToggleLog: () -> Unit,
    onRevoke: () -> Unit
) {
    val rotation by animateFloatAsState(if (item.expanded) 90f else 0f, label = "rotation")
    val isAllowed = item.policy >= SuPolicy.ALLOW
    val statusColor = when {
        isAllowed -> Color(0xFF4CAF50)
        item.policy == SuPolicy.RESTRICT -> Color(0xFFFF9800)
        else -> MaterialTheme.colorScheme.error
    }

    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .animateContentSize()
            .border(
                width = 1.dp,
                brush = Brush.linearGradient(listOf(statusColor.copy(alpha = 0.4f), Color.Transparent)),
                shape = RoundedCornerShape(topEnd = 48.dp, bottomStart = 48.dp, topStart = 12.dp, bottomEnd = 12.dp)
            ),
        shape = RoundedCornerShape(topEnd = 48.dp, bottomStart = 48.dp, topStart = 12.dp, bottomEnd = 12.dp),
        onClick = onToggleExpanded,
        colors = CardDefaults.elevatedCardColors(
            containerColor = if (item.expanded) MaterialTheme.colorScheme.surfaceContainerHighest else MaterialTheme.colorScheme.surfaceContainerLow
        )
    ) {
        Box {
            Icon(
                painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                contentDescription = null,
                modifier = Modifier
                    .size(120.dp)
                    .align(Alignment.TopEnd)
                    .offset(x = 30.dp, y = (-20).dp)
                    .alpha(0.05f),
                tint = MaterialTheme.colorScheme.primary
            )

            Column(modifier = Modifier.padding(24.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Box(contentAlignment = Alignment.BottomEnd) {
                        Surface(
                            modifier = Modifier.size(58.dp),
                            shape = RoundedCornerShape(18.dp),
                            color = MaterialTheme.colorScheme.surface,
                            tonalElevation = 3.dp
                        ) {
                            Image(
                                painter = BitmapPainter(item.icon.toBitmap().asImageBitmap()),
                                contentDescription = null,
                                modifier = Modifier.padding(10.dp)
                            )
                        }
                        Box(
                            modifier = Modifier
                                .size(18.dp)
                                .clip(CircleShape)
                                .background(statusColor)
                                .border(2.dp, MaterialTheme.colorScheme.surface, CircleShape)
                        )
                    }
                    
                    Column(modifier = Modifier.weight(1f).padding(horizontal = 16.dp)) {
                        Text(
                            text = item.title,
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.ExtraBold,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Surface(
                                color = statusColor.copy(alpha = 0.15f),
                                shape = RoundedCornerShape(8.dp)
                            ) {
                                Text(
                                    text = if (isAllowed) "AUTHORIZED" else if (item.policy == SuPolicy.RESTRICT) "RESTRICTED" else "DENIED",
                                    modifier = Modifier.padding(horizontal = 8.dp, vertical = 2.dp),
                                    style = MaterialTheme.typography.labelSmall,
                                    color = statusColor,
                                    fontWeight = FontWeight.Black,
                                    letterSpacing = 0.5.sp
                                )
                            }
                        }
                    }

                    if (item.showSlider) {
                        var sliderValue by remember(item.uid) { mutableStateOf(policyToSliderValue(item.policy)) }
                        Slider(
                            modifier = Modifier.width(80.dp),
                            value = sliderValue,
                            onValueChange = { sliderValue = it },
                            valueRange = 1f..3f,
                            steps = 1,
                            onValueChangeFinished = { onSliderFinished(sliderValue) }
                        )
                    } else {
                        Switch(
                            checked = isAllowed,
                            onCheckedChange = onToggleEnabled,
                            thumbContent = { if (isAllowed) Icon(Icons.Rounded.Check, null, Modifier.size(SwitchDefaults.IconSize)) }
                        )
                    }
                    
                    Icon(
                        imageVector = Icons.AutoMirrored.Rounded.NavigateNext,
                        contentDescription = null,
                        modifier = Modifier.rotate(rotation).padding(start = 8.dp),
                        tint = MaterialTheme.colorScheme.outline
                    )
                }

                AnimatedVisibility(
                    visible = item.expanded,
                    enter = expandVertically() + fadeIn(),
                    exit = shrinkVertically() + fadeOut()
                ) {
                    Column {
                        Spacer(Modifier.height(24.dp))
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(12.dp)
                        ) {
                            ExpressiveActionPill(Icons.Rounded.Notifications, "Notify", item.notification, onToggleNotify, Modifier.weight(1f))
                            ExpressiveActionPill(Icons.Rounded.BugReport, "Logs", item.logging, onToggleLog, Modifier.weight(1f))
                            ExpressiveActionPill(Icons.Rounded.Delete, "Revoke", false, onRevoke, Modifier.weight(1f), isError = true)
                        }
                        
                        Spacer(Modifier.height(16.dp))
                        Text(
                            text = "Package: ${item.packageName}",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f),
                            modifier = Modifier.fillMaxWidth(),
                            textAlign = TextAlign.Center
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ExpressiveActionPill(
    icon: ImageVector,
    label: String,
    active: Boolean,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    isError: Boolean = false
) {
    val containerColor = when {
        isError -> MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.3f)
        active -> MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.5f)
        else -> MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
    }
    val contentColor = when {
        isError -> MaterialTheme.colorScheme.error
        active -> MaterialTheme.colorScheme.primary
        else -> MaterialTheme.colorScheme.onSurfaceVariant
    }

    Surface(
        modifier = modifier.height(54.dp).clickable { onClick() },
        shape = RoundedCornerShape(18.dp),
        color = containerColor,
        border = if (active || isError) androidx.compose.foundation.BorderStroke(1.dp, contentColor.copy(alpha = 0.2f)) else null
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Center
        ) {
            Icon(icon, null, modifier = Modifier.size(20.dp), tint = contentColor)
            Spacer(Modifier.width(8.dp))
            Text(label, style = MaterialTheme.typography.labelMedium, fontWeight = FontWeight.Black, color = contentColor)
        }
    }
}

// Logic components remain identical
data class PolicyUiItem(val uid: Int, val packageName: String, val appName: String, val icon: Drawable, val isSharedUid: Boolean, val policy: Int, val notification: Boolean, val logging: Boolean, val expanded: Boolean = false) {
    val title: String get() = if (isSharedUid) "[SharedUID] $appName" else appName
    val showSlider: Boolean get() = Config.suRestrict || policy == SuPolicy.RESTRICT
}

data class SuperuserUiState(val loading: Boolean = true, val items: List<PolicyUiItem> = emptyList(), val message: String? = null)

class SuperuserComposeViewModel(private val policyDao: com.topjohnwu.magisk.core.data.magiskdb.PolicyDao, private val pm: PackageManager) : ViewModel() {
    private val _state = MutableStateFlow(SuperuserUiState())
    val state: StateFlow<SuperuserUiState> = _state
    init { reload() }
    fun reload() {
        _state.update { it.copy(loading = true) }
        viewModelScope.launch {
            val policies = loadPolicies()
            _state.update { it.copy(loading = false, items = policies) }
        }
    }
    private suspend fun loadPolicies(): List<PolicyUiItem> = withContext(Dispatchers.IO) {
        if (!Info.showSuperUser) return@withContext emptyList()
        policyDao.deleteOutdated(); policyDao.delete(AppContext.applicationInfo.uid)
        val policies = mutableListOf<PolicyUiItem>()
        for (policy in policyDao.fetchAll()) {
            val pkgs = if (policy.uid == Process.SYSTEM_UID) arrayOf("android") else pm.getPackagesForUid(policy.uid)
            pkgs?.forEach { pkg ->
                try {
                    val info = pm.getPackageInfo(pkg, PackageManager.MATCH_UNINSTALLED_PACKAGES)
                    val appInfo = info.applicationInfo
                    policies.add(PolicyUiItem(policy.uid, info.packageName, appInfo?.loadLabel(pm)?.toString() ?: info.packageName, appInfo?.loadIcon(pm) ?: pm.defaultActivityIcon, info.sharedUserId != null, policy.policy, policy.notification, policy.logging))
                } catch (_: Exception) {}
            }
        }
        policies.sortedBy { it.appName.lowercase(Locale.ROOT) }
    }
    fun toggleExpanded(uid: Int, pkg: String) { _state.update { st -> st.copy(items = st.items.map { if (it.uid == uid && it.packageName == pkg) it.copy(expanded = !it.expanded) else it }) } }
    fun setPolicy(uid: Int, p: Int) {
        viewModelScope.launch {
            val item = state.value.items.firstOrNull { it.uid == uid } ?: return@launch
            withContext(Dispatchers.IO) { policyDao.update(SuPolicy(uid).apply { policy = p; notification = item.notification; logging = item.logging }) }
            _state.update { st -> st.copy(items = st.items.map { if (it.uid == uid) it.copy(policy = p) else it }, message = AppContext.getString(if (p >= SuPolicy.ALLOW) CoreR.string.su_snack_grant else CoreR.string.su_snack_deny, item.appName)) }
        }
    }
    fun toggleNotify(item: PolicyUiItem) {
        viewModelScope.launch {
            val uid = item.uid
            val nv = !item.notification
            withContext(Dispatchers.IO) { policyDao.update(SuPolicy(uid).apply { policy = item.policy; notification = nv; logging = item.logging }) }
            _state.update { st -> st.copy(items = st.items.map { if (it.uid == uid) it.copy(notification = nv) else it }, message = AppContext.getString(if (nv) CoreR.string.su_snack_notif_on else CoreR.string.su_snack_notif_off, item.appName)) }
        }
    }
    fun toggleLog(item: PolicyUiItem) {
        viewModelScope.launch {
            val uid = item.uid
            val nv = !item.logging
            withContext(Dispatchers.IO) { policyDao.update(SuPolicy(uid).apply { policy = item.policy; notification = item.notification; logging = nv }) }
            _state.update { st -> st.copy(items = st.items.map { if (it.uid == uid) it.copy(logging = nv) else it }, message = AppContext.getString(if (nv) CoreR.string.su_snack_log_on else CoreR.string.su_snack_log_off, item.appName)) }
        }
    }
    fun revoke(item: PolicyUiItem) {
        viewModelScope.launch {
            val uid = item.uid
            withContext(Dispatchers.IO) { policyDao.delete(uid) }
            _state.update { st -> st.copy(items = st.items.filterNot { it.uid == uid }, message = AppContext.getString(CoreR.string.su_snack_deny, item.appName)) }
        }
    }
    fun consumeMessage() { _state.update { it.copy(message = null) } }
    object Factory : ViewModelProvider.Factory { override fun <T : ViewModel> create(modelClass: Class<T>): T { @Suppress("UNCHECKED_CAST") return SuperuserComposeViewModel(ServiceLocator.policyDB, AppContext.packageManager) as T } }
}

private fun policyToSliderValue(p: Int): Float = when (p) { SuPolicy.DENY -> 1f; SuPolicy.RESTRICT -> 2f; SuPolicy.ALLOW -> 3f; else -> 1f }
private fun sliderValueToPolicy(v: Float): Int = when (v.toInt()) { 1 -> SuPolicy.DENY; 2 -> SuPolicy.RESTRICT; 3 -> SuPolicy.ALLOW; else -> SuPolicy.DENY }
