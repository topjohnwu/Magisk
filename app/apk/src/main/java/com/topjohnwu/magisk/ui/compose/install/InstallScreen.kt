package com.topjohnwu.magisk.ui.compose.install

import android.net.Uri
import android.os.Build
import android.widget.TextView
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts.OpenDocument
import androidx.compose.animation.*
import androidx.compose.animation.core.spring
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ArrowForward
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
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
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.persistReadPermission
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.collect
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
        if (uri != null) {
            uri.persistReadPermission()
            viewModel.setPatchUri(uri)
        }
    }

    LaunchedEffect(viewModel) {
        viewModel.messages.collect { snackbarHostState.showSnackbar(it) }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 140.dp, start = 20.dp, end = 20.dp, top = 12.dp),
            verticalArrangement = Arrangement.spacedBy(28.dp)
        ) {
            if (!state.skipOptions) {
                item {
                    InstallSection(
                        title = stringResource(id = CoreR.string.install_options_title),
                        icon = Icons.Rounded.Tune
                    ) {
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
                            Box(modifier = Modifier.padding(16.dp)) {
                                Button(
                                    onClick = viewModel::nextStep,
                                    modifier = Modifier.fillMaxWidth().height(56.dp),
                                    shape = RoundedCornerShape(16.dp),
                                    colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)
                                ) {
                                    Text(
                                        stringResource(id = CoreR.string.install_next),
                                        style = MaterialTheme.typography.titleMedium,
                                        fontWeight = FontWeight.ExtraBold
                                    )
                                    Spacer(Modifier.width(8.dp))
                                    Icon(Icons.AutoMirrored.Rounded.ArrowForward, null, modifier = Modifier.size(20.dp))
                                }
                            }
                        }
                    }
                }
            }

            item {
                InstallSection(
                    title = stringResource(id = CoreR.string.install_method_title),
                    icon = Icons.Rounded.SettingsSuggest
                ) {
                    AnimatedContent(
                        targetState = state.step >= 1,
                        transitionSpec = { fadeIn() togetherWith fadeOut() },
                        label = "methodContent"
                    ) { isStepReady ->
                        if (isStepReady) {
                            Column {
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
                                
                                Box(modifier = Modifier.padding(16.dp)) {
                                    Button(
                                        onClick = {
                                            val request = viewModel.buildFlashRequest() ?: return@Button
                                            onStartFlash(request.action, request.uri)
                                        },
                                        enabled = state.canStart,
                                        modifier = Modifier
                                            .fillMaxWidth()
                                            .height(64.dp),
                                        shape = RoundedCornerShape(20.dp),
                                        contentPadding = PaddingValues(horizontal = 24.dp)
                                    ) {
                                        Icon(Icons.Rounded.DownloadDone, null)
                                        Spacer(Modifier.width(12.dp))
                                        Text(
                                            stringResource(id = CoreR.string.install_start),
                                            style = MaterialTheme.typography.titleLarge,
                                            fontWeight = FontWeight.Black
                                        )
                                    }
                                }
                            }
                        } else {
                            Box(modifier = Modifier.padding(24.dp).fillMaxWidth(), contentAlignment = Alignment.Center) {
                                Text(
                                    stringResource(id = CoreR.string.install_complete_options_first),
                                    style = MaterialTheme.typography.bodyLarge,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f),
                                    textAlign = TextAlign.Center
                                )
                            }
                        }
                    }
                }
            }

            if (state.notes.isNotBlank()) {
                item {
                    InstallSection(
                        title = stringResource(id = CoreR.string.release_notes),
                        icon = Icons.Rounded.HistoryEdu
                    ) {
                        MarkdownText(
                            markdown = state.notes,
                            modifier = Modifier.padding(20.dp)
                        )
                    }
                }
            }
        }
    }

    if (showSlotWarning) {
        AlertDialog(
            onDismissRequest = { showSlotWarning = false },
            title = { 
                Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                    Surface(color = MaterialTheme.colorScheme.errorContainer, shape = CircleShape, modifier = Modifier.size(32.dp)) {
                        Icon(Icons.Rounded.Warning, null, modifier = Modifier.padding(8.dp), tint = MaterialTheme.colorScheme.onErrorContainer)
                    }
                    Text(
                        text = stringResource(id = CoreR.string.install_inactive_slot),
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis
                    )
                }
            },
            text = { 
                Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text(stringResource(id = CoreR.string.install_inactive_slot_msg), style = MaterialTheme.typography.bodyMedium)
                    Surface(
                        color = MaterialTheme.colorScheme.error.copy(alpha = 0.05f),
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            text = stringResource(id = CoreR.string.install_inactive_slot_caution),
                            modifier = Modifier.padding(12.dp),
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.error,
                            fontWeight = FontWeight.Bold,
                            textAlign = TextAlign.Center
                        )
                    }
                }
            },
            shape = RoundedCornerShape(32.dp),
            confirmButton = {
                Button(
                    onClick = {
                        viewModel.setMethod(InstallMethod.InactiveSlot)
                        showSlotWarning = false
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error)
                ) { Text(stringResource(id = android.R.string.ok), fontWeight = FontWeight.Black) }
            },
            dismissButton = {
                TextButton(onClick = { showSlotWarning = false }) { Text(stringResource(id = android.R.string.cancel)) }
            },
            containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
        )
    }
}

@Composable
private fun InstallSection(
    title: String,
    icon: ImageVector,
    content: @Composable ColumnScope.() -> Unit
) {
    Column(modifier = Modifier.animateContentSize(spring(stiffness = androidx.compose.animation.core.Spring.StiffnessLow))) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.padding(horizontal = 8.dp, vertical = 12.dp)
        ) {
            Surface(
                color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f),
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier.size(32.dp)
            ) {
                Icon(
                    icon, null,
                    modifier = Modifier.padding(6.dp),
                    tint = MaterialTheme.colorScheme.onPrimaryContainer
                )
            }
            Spacer(Modifier.width(16.dp))
            Text(
                text = title.uppercase(),
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Black,
                letterSpacing = 1.2.sp,
                color = MaterialTheme.colorScheme.outline
            )
        }
        
        ElevatedCard(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(28.dp),
            colors = CardDefaults.elevatedCardColors(
                containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
            ),
            elevation = CardDefaults.elevatedCardElevation(defaultElevation = 2.dp)
        ) {
            Column(
                modifier = Modifier.padding(vertical = 8.dp),
                content = content
            )
        }
    }
}

@Composable
private fun ExpressiveToggleRow(title: String, checked: Boolean, visible: Boolean, onToggle: (Boolean) -> Unit) {
    if (!visible) return
    Surface(
        color = Color.Transparent,
        modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(16.dp)).clickable { onToggle(!checked) }
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 24.dp, vertical = 18.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                title, 
                modifier = Modifier.weight(1f), 
                style = MaterialTheme.typography.titleMedium, 
                fontWeight = FontWeight.Bold,
                color = MaterialTheme.colorScheme.onSurface
            )
            Switch(
                checked = checked, 
                onCheckedChange = onToggle,
                thumbContent = if (checked) {
                    { Icon(Icons.Rounded.Check, null, Modifier.size(16.dp)) }
                } else null
            )
        }
    }
}

@Composable
private fun ExpressiveMethodRow(
    title: String, 
    subtitle: String, 
    selected: Boolean, 
    icon: ImageVector, 
    onClick: () -> Unit
) {
    val backgroundColor by animateColorAsState(
        if (selected) MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.30f)
        else Color.Transparent,
        label = "bg"
    )

    Surface(
        color = backgroundColor,
        shape = RoundedCornerShape(20.dp),
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp, vertical = 4.dp)
            .clip(RoundedCornerShape(20.dp))
            .clickable { onClick() }
    ) {
        Row(
            modifier = Modifier.padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Surface(
                color = if (selected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surfaceVariant,
                shape = CircleShape,
                modifier = Modifier.size(48.dp)
            ) {
                Icon(
                    icon, null,
                    modifier = Modifier.padding(12.dp),
                    tint = if (selected) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Spacer(Modifier.width(16.dp))
            Column(Modifier.weight(1f)) {
                Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Text(
                        title,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.ExtraBold,
                        color = if (selected) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.onSurfaceVariant,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                }
                Text(
                    subtitle,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.8f),
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )
            }
            RadioButton(
                selected = selected,
                onClick = null,
                colors = RadioButtonDefaults.colors(selectedColor = MaterialTheme.colorScheme.primary)
            )
        }
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
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    lineHeight = (textSize * 1.4f).toInt()
                }
            }
        },
        update = { textView ->
            ServiceLocator.markwon.setMarkdown(textView, markdown)
        }
    )
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
    val notes: String = ""
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
    private val _messages = MutableSharedFlow<String>(extraBufferCapacity = 1)
    val messages: SharedFlow<String> = _messages.asSharedFlow()

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
            _messages.tryEmit(AppContext.getString(CoreR.string.patch_file_msg))
            return null
        }
        return FlashRequest(method.action, state.patchUri)
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
