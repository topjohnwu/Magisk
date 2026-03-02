package com.topjohnwu.magisk.ui.compose.install

import android.net.Uri
import android.widget.TextView
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts.OpenDocument
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun InstallScreen(
    onStartFlash: (action: String, uri: Uri?) -> Unit
) {
    val viewModel: InstallComposeViewModel = viewModel(factory = InstallComposeViewModel.Factory)
    val state = viewModel.state
    val snackbarHostState = remember { SnackbarHostState() }
    var showSlotWarning by remember { mutableStateOf(false) }

    val patchPicker = rememberLauncherForActivityResult(OpenDocument()) { uri ->
        if (uri != null) viewModel.setPatchUri(uri)
    }

    LaunchedEffect(state.message) {
        state.message?.let {
            snackbarHostState.showSnackbar(it)
            viewModel.consumeMessage()
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 120.dp, start = 24.dp, end = 24.dp, top = 8.dp),
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            if (!state.skipOptions) {
                item {
                    InstallSection(stringResource(id = CoreR.string.install_options_title), Icons.Rounded.Tune) {
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.keep_dm_verity),
                            checked = state.keepVerity,
                            visible = !Info.isSAR,
                            onToggle = viewModel::setKeepVerity
                        )
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.keep_force_encryption),
                            checked = state.keepEnc,
                            visible = Info.isFDE,
                            onToggle = viewModel::setKeepEnc
                        )
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.recovery_mode),
                            checked = state.recovery,
                            visible = !Info.ramdisk,
                            onToggle = viewModel::setRecovery
                        )
                        if (state.step == 0) {
                            Spacer(Modifier.height(12.dp))
                            Button(
                                onClick = viewModel::nextStep,
                                modifier = Modifier.fillMaxWidth().height(56.dp).padding(horizontal = 20.dp),
                                shape = RoundedCornerShape(16.dp)
                            ) {
                                Text(stringResource(id = CoreR.string.install_next), fontWeight = FontWeight.Bold)
                            }
                            Spacer(Modifier.height(8.dp))
                        }
                    }
                }
            }

            item {
                InstallSection(stringResource(id = CoreR.string.install_method_title), Icons.Rounded.SettingsSuggest) {
                    if (state.step >= 1) {
                        ExpressiveMethodRow(
                            title = stringResource(id = CoreR.string.select_patch_file),
                            subtitle = if (state.patchUri != null && state.method == InstallMethod.Patch) {
                                state.patchUri.displayName
                            } else {
                                stringResource(id = CoreR.string.install_select_patch_file_subtitle)
                            },
                            selected = state.method == InstallMethod.Patch,
                            icon = Icons.Rounded.FileCopy,
                            onClick = {
                                viewModel.setMethod(InstallMethod.Patch)
                                patchPicker.launch(arrayOf("*/*"))
                            }
                        )
                        if (state.isRooted) {
                            ExpressiveMethodRow(
                                title = stringResource(id = CoreR.string.direct_install),
                                subtitle = stringResource(id = CoreR.string.install_direct_install_subtitle),
                                selected = state.method == InstallMethod.Direct,
                                icon = Icons.Rounded.FlashOn,
                                onClick = { viewModel.setMethod(InstallMethod.Direct) }
                            )
                        }
                        if (!state.noSecondSlot) {
                            ExpressiveMethodRow(
                                title = stringResource(id = CoreR.string.install_inactive_slot),
                                subtitle = stringResource(id = CoreR.string.install_inactive_slot_subtitle),
                                selected = state.method == InstallMethod.InactiveSlot,
                                icon = Icons.Rounded.DynamicFeed,
                                onClick = { showSlotWarning = true }
                            )
                        }
                        
                        Spacer(Modifier.height(16.dp))
                        Button(
                            onClick = {
                                val request = viewModel.buildFlashRequest() ?: return@Button
                                onStartFlash(request.action, request.uri)
                            },
                            enabled = state.canStart,
                            modifier = Modifier.fillMaxWidth().height(60.dp).padding(horizontal = 20.dp),
                            shape = RoundedCornerShape(20.dp)
                        ) {
                            Icon(Icons.Rounded.InstallDesktop, null)
                            Spacer(Modifier.width(12.dp))
                            Text(stringResource(id = CoreR.string.install_start), fontWeight = FontWeight.Black)
                        }
                        Spacer(Modifier.height(8.dp))
                    } else {
                        Box(modifier = Modifier.padding(20.dp)) {
                            Text(
                                stringResource(id = CoreR.string.install_complete_options_first),
                                style = MaterialTheme.typography.bodyMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f)
                            )
                        }
                    }
                }
            }

            if (state.notes.isNotBlank()) {
                item {
                    InstallSection(stringResource(id = CoreR.string.release_notes), Icons.Rounded.HistoryEdu) {
                        MarkdownText(
                            markdown = state.notes,
                            modifier = Modifier.padding(20.dp)
                        )
                    }
                }
            }
        }

        SnackbarHost(
            hostState = snackbarHostState,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 110.dp)
        )
    }

    if (showSlotWarning) {
        AlertDialog(
            onDismissRequest = { showSlotWarning = false },
            title = { Text(stringResource(id = CoreR.string.install_inactive_slot), fontWeight = FontWeight.Black) },
            text = { Text(stringResource(id = CoreR.string.install_inactive_slot_msg)) },
            shape = RoundedCornerShape(28.dp),
            confirmButton = {
                TextButton(onClick = {
                    viewModel.setMethod(InstallMethod.InactiveSlot)
                    showSlotWarning = false
                }) { Text(stringResource(id = android.R.string.ok), fontWeight = FontWeight.Bold) }
            },
            dismissButton = {
                TextButton(onClick = { showSlotWarning = false }) { Text(stringResource(id = android.R.string.cancel)) }
            }
        )
    }
}

@Composable
private fun MarkdownText(
    markdown: String,
    modifier: Modifier = Modifier
) {
    val textColor = MaterialTheme.colorScheme.onSurfaceVariant.toArgb()
    AndroidView(
        modifier = modifier,
        factory = { context ->
            TextView(context).apply {
                setTextColor(textColor)
                textSize = 14f
            }
        },
        update = { textView ->
            ServiceLocator.markwon.setMarkdown(textView, markdown)
        }
    )
}

@Composable
private fun InstallSection(title: String, icon: ImageVector, content: @Composable ColumnScope.() -> Unit) {
    Column {
        Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(start = 8.dp, bottom = 12.dp)) {
            Icon(icon, null, modifier = Modifier.size(20.dp), tint = MaterialTheme.colorScheme.primary)
            Spacer(Modifier.width(12.dp))
            Text(
                text = title.uppercase(),
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Black,
                letterSpacing = 1.5.sp,
                color = MaterialTheme.colorScheme.outline
            )
        }
        ElevatedCard(
            shape = RoundedCornerShape(topEnd = 40.dp, bottomStart = 40.dp, topStart = 12.dp, bottomEnd = 12.dp),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
        ) {
            Column(modifier = Modifier.padding(vertical = 8.dp), content = content)
        }
    }
}

@Composable
private fun ExpressiveToggleRow(title: String, checked: Boolean, visible: Boolean, onToggle: (Boolean) -> Unit) {
    if (!visible) return
    Row(
        modifier = Modifier.fillMaxWidth().clickable { onToggle(!checked) }.padding(20.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(title, modifier = Modifier.weight(1f), style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Bold)
        Switch(checked = checked, onCheckedChange = onToggle)
    }
}

@Composable
private fun ExpressiveMethodRow(title: String, subtitle: String, selected: Boolean, icon: ImageVector, onClick: () -> Unit) {
    Row(
        modifier = Modifier.fillMaxWidth().clickable { onClick() }.padding(20.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Surface(
            color = if (selected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.primary.copy(alpha = 0.1f),
            shape = CircleShape,
            modifier = Modifier.size(44.dp)
        ) {
            Icon(icon, null, modifier = Modifier.padding(10.dp), tint = if (selected) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.primary)
        }
        Spacer(Modifier.width(20.dp))
        Column(Modifier.weight(1f)) {
            Text(title, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Bold)
            Text(subtitle, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant, maxLines = 1, overflow = TextOverflow.Ellipsis)
        }
        RadioButton(selected = selected, onClick = null)
    }
    HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.14f))
}

private enum class InstallMethod(
    val action: String,
    val needsUri: Boolean
) {
    Patch(Const.Value.PATCH_FILE, true),
    Direct(Const.Value.FLASH_MAGISK, false),
    InactiveSlot(Const.Value.FLASH_INACTIVE_SLOT, false)
}

private data class FlashRequest(
    val action: String,
    val uri: Uri?
)

private data class InstallUiState(
    val step: Int = if (skipOptions()) 1 else 0,
    val skipOptions: Boolean = skipOptions(),
    val noSecondSlot: Boolean = noSecondSlot(),
    val isRooted: Boolean = Info.isRooted,
    val keepVerity: Boolean = Config.keepVerity,
    val keepEnc: Boolean = Config.keepEnc,
    val recovery: Boolean = Config.recovery,
    val method: InstallMethod? = null,
    val patchUri: Uri? = null,
    val notes: String = "",
    val message: String? = null
) {
    val canStart: Boolean
        get() = method != null && (!method.needsUri || patchUri != null)

    companion object {
        private fun skipOptions() = Info.isEmulator || (Info.isSAR && !Info.isFDE && Info.ramdisk)
        private fun noSecondSlot() = !Info.isRooted || !Info.isAB || Info.isEmulator
    }
}

private class InstallComposeViewModel(
    private val svc: NetworkService
) : ViewModel() {

    var state by mutableStateOf(InstallUiState())
        private set

    init {
        loadNotes()
    }

    fun nextStep() {
        state = state.copy(step = 1)
    }

    fun setKeepVerity(value: Boolean) {
        Config.keepVerity = value
        state = state.copy(keepVerity = value)
    }

    fun setKeepEnc(value: Boolean) {
        Config.keepEnc = value
        state = state.copy(keepEnc = value)
    }

    fun setRecovery(value: Boolean) {
        Config.recovery = value
        state = state.copy(recovery = value)
    }

    fun setMethod(method: InstallMethod) {
        state = state.copy(method = method)
    }

    fun setPatchUri(uri: Uri) {
        state = state.copy(patchUri = uri)
    }

    fun buildFlashRequest(): FlashRequest? {
        val method = state.method ?: return null
        if (method.needsUri && state.patchUri == null) {
            state = state.copy(message = AppContext.getString(CoreR.string.patch_file_msg))
            return null
        }
        return FlashRequest(method.action, state.patchUri)
    }

    fun consumeMessage() {
        state = state.copy(message = null)
    }

    private fun loadNotes() {
        viewModelScope.launch {
            val note = withContext(Dispatchers.IO) {
                runCatching {
                    val noteFile = File(AppContext.cacheDir, "${BuildConfig.APP_VERSION_CODE}.md")
                    when {
                        noteFile.exists() -> noteFile.readText()
                        else -> {
                            val remote = svc.fetchUpdate(BuildConfig.APP_VERSION_CODE)?.note.orEmpty()
                            if (remote.isNotBlank()) {
                                noteFile.writeText(remote)
                            }
                            remote
                        }
                    }
                }.getOrDefault("")
            }
            state = state.copy(notes = note)
        }
    }

    companion object {
        val Factory = object : ViewModelProvider.Factory {
            override fun <T : ViewModel> create(modelClass: Class<T>): T {
                @Suppress("UNCHECKED_CAST")
                return InstallComposeViewModel(ServiceLocator.networkService) as T
            }
        }
    }
}
