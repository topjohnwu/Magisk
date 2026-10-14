package com.topjohnwu.magisk.ui.install

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbManager
import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.adaptive.currentWindowAdaptiveInfo
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import android.graphics.Typeface
import android.text.Spanned
import android.text.style.StyleSpan
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.core.content.ContextCompat
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.window.core.layout.WindowSizeClass
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.repository.FirmwareCrawler
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import com.topjohnwu.magisk.core.utils.UsbBlockDevice
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.MagiskDialog
import com.topjohnwu.magisk.ui.component.MarkdownText
import com.topjohnwu.magisk.ui.component.SettingsArrow
import com.topjohnwu.magisk.ui.component.SettingsSwitch
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun InstallDialog(
    show: Boolean,
    onDismiss: () -> Unit,
    installVm: InstallViewModel,
    modifier: Modifier = Modifier
) {
    val installUiState by installVm.uiState.collectAsStateWithLifecycle()
    var showPatchDialog by rememberSaveable { mutableStateOf(false) }

    val secondSlotDialog = rememberConfirmDialog()
    val secondSlotTitle = stringResource(android.R.string.dialog_alert_title)
    val secondSlotMsg = stringResource(CoreR.string.install_inactive_slot_msg)

    LaunchedEffect(installUiState.showSecondSlotWarning) {
        if (installUiState.showSecondSlotWarning) {
            val result = secondSlotDialog.awaitConfirm(title = secondSlotTitle, content = secondSlotMsg)
            installVm.onSecondSlotWarningConsumed()
            if (result == ConfirmResult.Confirmed) {
                installVm.install()
            }
        }
    }

    LaunchedEffect(installUiState.showPatchDialog) {
        if (installUiState.showPatchDialog) {
            showPatchDialog = true
            installVm.onPatchDialogConsumed()
        }
    }

    if (showPatchDialog) {
        PatchComposableDialog(
            candidate = installUiState.candidate,
            showUsbDestination = installVm.isGooglebook,
            onDismiss = { showPatchDialog = false },
            onConfirm = { uri, usbDevice ->
                showPatchDialog = false
                installVm.onPatchSelected(uri, usbDevice)
            }
        )
    }

    // Full screen dialogs are only for compact windows; read this outside of the Dialog window
    val fullScreen = !currentWindowAdaptiveInfo().windowSizeClass
        .isWidthAtLeastBreakpoint(WindowSizeClass.WIDTH_DP_MEDIUM_LOWER_BOUND)

    if (show) {
        Dialog(
            onDismissRequest = onDismiss,
            properties = DialogProperties(
                usePlatformDefaultWidth = false,
                decorFitsSystemWindows = false
            )
        ) {
            val scrollState = rememberScrollState()
            Scaffold(
                modifier = if (fullScreen) modifier.fillMaxSize() else modifier
                    .windowInsetsPadding(WindowInsets.systemBars)
                    .padding(24.dp)
                    .widthIn(max = 560.dp)
                    .heightIn(max = 800.dp)
                    .clip(RoundedCornerShape(28.dp)),
                containerColor = if (fullScreen) MaterialTheme.colorScheme.background
                    else MaterialTheme.colorScheme.surfaceContainerHigh,
                topBar = {
                    TopAppBar(
                        title = { Text(stringResource(CoreR.string.install)) },
                        navigationIcon = {
                            IconButton(onClick = onDismiss) {
                                Icon(
                                    imageVector = Icons.Default.Close,
                                    contentDescription = stringResource(android.R.string.cancel)
                                )
                            }
                        },
                        colors = TopAppBarDefaults.topAppBarColors(
                            containerColor = if (fullScreen) MaterialTheme.colorScheme.surface
                                else MaterialTheme.colorScheme.surfaceContainerHigh
                        )
                    )
                }
            ) { innerPadding ->
                Column(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(innerPadding)
                        .verticalScrollbar(scrollState, contentPadding = PaddingValues(vertical = 12.dp))
                        .verticalScroll(scrollState)
                        .padding(horizontal = 16.dp, vertical = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    if (installUiState.notes.isNotEmpty()) {
                        MarkdownText(installUiState.notes)
                    }

                    if (!installVm.skipOptions) {
                        InstallOptionsSection()
                    }

                    Card(
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(20.dp),
                        colors = CardDefaults.cardColors(
                            containerColor = MaterialTheme.colorScheme.surfaceContainerLow
                        ),
                    ) {
                        SettingsArrow(
                            title = stringResource(CoreR.string.patch_image),
                            onClick = {
                                onDismiss()
                                installVm.selectMethod(InstallViewModel.Method.PATCH)
                            },
                        )

                        if (installVm.isRooted) {
                            SettingsArrow(
                                title = stringResource(CoreR.string.direct_install),
                                onClick = {
                                    onDismiss()
                                    installVm.selectMethod(InstallViewModel.Method.DIRECT)
                                    installVm.install()
                                },
                            )
                        }

                        if (!installVm.noSecondSlot) {
                            SettingsArrow(
                                title = stringResource(CoreR.string.install_inactive_slot),
                                onClick = {
                                    onDismiss()
                                    installVm.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                                },
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun InstallOptionsSection(
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
    ) {
        if (!Info.isSAR) {
            var keepVerity by remember { mutableStateOf(Config.keepVerity) }
            SettingsSwitch(
                title = stringResource(CoreR.string.keep_dm_verity),
                checked = keepVerity,
                onCheckedChange = {
                    keepVerity = it
                    Config.keepVerity = it
                }
            )
        }
        if (Info.isFDE) {
            var keepEnc by remember { mutableStateOf(Config.keepEnc) }
            SettingsSwitch(
                title = stringResource(CoreR.string.keep_force_encryption),
                checked = keepEnc,
                onCheckedChange = {
                    keepEnc = it
                    Config.keepEnc = it
                }
            )
        }
        if (!Info.ramdisk) {
            var recovery by remember { mutableStateOf(Config.recovery) }
            SettingsSwitch(
                title = stringResource(CoreR.string.recovery_mode),
                checked = recovery,
                onCheckedChange = {
                    recovery = it
                    Config.recovery = it
                }
            )
        }
    }
}

private fun parseHttpsUrl(url: String): Uri? {
    if (url.isEmpty()) return null
    val uri = url.toUri()
    if (!uri.scheme.equals("https", ignoreCase = true)) return null
    if (uri.host.isNullOrEmpty()) return null
    if (uri.path.isNullOrEmpty()) return null
    return uri
}

private enum class PatchSource {
    CANDIDATE, FILE, URL
}

@Composable
fun PatchComposableDialog(
    onDismiss: () -> Unit,
    onConfirm: (Uri, String?) -> Unit,
    modifier: Modifier = Modifier,
    candidate: FirmwareCrawler.Candidate? = null,
    showUsbDestination: Boolean = false,
) {
    val context = LocalContext.current
    var source by rememberSaveable(candidate) {
        mutableStateOf(if (candidate != null) PatchSource.CANDIDATE else PatchSource.FILE)
    }
    var localFileUri by rememberSaveable { mutableStateOf<Uri?>(null) }
    var pendingSubmitAfterPick by rememberSaveable { mutableStateOf(false) }
    var url by rememberSaveable { mutableStateOf("") }
    var isUrlError by rememberSaveable { mutableStateOf(false) }

    var usbDevices by remember {
        mutableStateOf(
            if (showUsbDestination) UsbBlockDevice.listDevices(context) else emptyList()
        )
    }
    var selectedUsb by rememberSaveable { mutableStateOf<String?>(null) }
    var preferFile by rememberSaveable { mutableStateOf(false) }

    if (showUsbDestination) {
        DisposableEffect(context) {
            usbDevices = UsbBlockDevice.listDevices(context)
            val receiver = object : BroadcastReceiver() {
                override fun onReceive(ctx: Context, intent: Intent) {
                    usbDevices = UsbBlockDevice.listDevices(context)
                }
            }
            val filter = IntentFilter().apply {
                addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED)
                addAction(UsbManager.ACTION_USB_DEVICE_DETACHED)
            }
            ContextCompat.registerReceiver(context, receiver, filter, ContextCompat.RECEIVER_EXPORTED)
            onDispose {
                runCatching { context.unregisterReceiver(receiver) }
            }
        }

        LaunchedEffect(usbDevices) {
            if (selectedUsb != null && usbDevices.none { it.deviceName == selectedUsb }) {
                selectedUsb = usbDevices.firstOrNull()?.deviceName
            } else if (selectedUsb == null && !preferFile && usbDevices.isNotEmpty()) {
                selectedUsb = usbDevices.first().deviceName
            }
        }
    }

    val confirmWithUri = { targetUri: Uri ->
        val usbTarget = if (showUsbDestination) {
            selectedUsb?.takeIf { name -> usbDevices.any { it.deviceName == name } }
        } else {
            null
        }
        onConfirm(targetUri, usbTarget)
    }

    val localFilePicker = rememberLauncherForActivityResult(ActivityResultContracts.GetContent()) { uri ->
        val shouldSubmit = pendingSubmitAfterPick
        pendingSubmitAfterPick = false
        if (uri != null) {
            localFileUri = uri
            source = PatchSource.FILE
            if (shouldSubmit) {
                confirmWithUri(uri)
            }
        }
    }

    val submit = submit@{
        val targetUri = when (source) {
            PatchSource.CANDIDATE -> candidate?.url?.toUri() ?: return@submit
            PatchSource.FILE -> localFileUri ?: run {
                pendingSubmitAfterPick = true
                localFilePicker.launch("*/*")
                return@submit
            }
            PatchSource.URL -> parseHttpsUrl(url.trim()) ?: run {
                isUrlError = true
                return@submit
            }
        }
        confirmWithUri(targetUri)
    }

    MagiskDialog(
        modifier = modifier,
        onDismissRequest = onDismiss,
        title = stringResource(CoreR.string.patch_image),
        confirmText = stringResource(android.R.string.ok),
        onConfirm = submit,
        dismissText = stringResource(android.R.string.cancel),
        onDismiss = onDismiss,
        scrollable = true,
    ) {
        Column(
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            val resources = LocalResources.current
            val boldColor = MaterialTheme.colorScheme.onSurface
            val lines = remember(resources, boldColor) {
                val cs = resources.getText(CoreR.string.patch_image_msg)
                val full = buildAnnotatedString {
                    append(cs.toString())
                    if (cs is Spanned) {
                        for (span in cs.getSpans(0, cs.length, StyleSpan::class.java)) {
                            if (span.style == Typeface.BOLD || span.style == Typeface.BOLD_ITALIC) {
                                addStyle(
                                    SpanStyle(fontWeight = FontWeight.SemiBold, color = boldColor),
                                    cs.getSpanStart(span),
                                    cs.getSpanEnd(span)
                                )
                            }
                        }
                    }
                }
                buildList {
                    var start = 0
                    while (start <= full.length) {
                        val end = full.indexOf('\n', start).let { if (it == -1) full.length else it }
                        add(full.subSequence(start, end))
                        start = end + 1
                    }
                }
            }

            Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
                lines.forEach { line ->
                    if (line.startsWith("• ")) {
                        Row(modifier = Modifier.fillMaxWidth()) {
                            Text(
                                text = "•",
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                modifier = Modifier.padding(start = 4.dp, end = 8.dp)
                            )
                            Text(
                                text = line.subSequence(2, line.length),
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    } else if (line.isNotEmpty()) {
                        Text(
                            text = line,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            HorizontalDivider(
                color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f)
            )

            Column(modifier = Modifier.selectableGroup()) {
                if (candidate != null) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(12.dp))
                            .selectable(
                                selected = source == PatchSource.CANDIDATE,
                                onClick = { source = PatchSource.CANDIDATE },
                                role = Role.RadioButton
                            )
                            .padding(vertical = 10.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = source == PatchSource.CANDIDATE,
                            onClick = null
                        )
                        Spacer(Modifier.width(16.dp))
                        Column {
                            Text(
                                text = candidate.title,
                                style = MaterialTheme.typography.bodyLarge,
                                color = MaterialTheme.colorScheme.onSurface
                            )
                            if (candidate.description.isNotEmpty()) {
                                Text(
                                    text = candidate.description,
                                    style = MaterialTheme.typography.bodySmall,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant
                                )
                            }
                        }
                    }
                }

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .selectable(
                            selected = source == PatchSource.FILE,
                            onClick = {
                                source = PatchSource.FILE
                                pendingSubmitAfterPick = false
                                localFilePicker.launch("*/*")
                            },
                            role = Role.RadioButton
                        )
                        .padding(vertical = 10.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = source == PatchSource.FILE,
                        onClick = null
                    )
                    Spacer(Modifier.width(16.dp))
                    Column {
                        Text(
                            text = stringResource(CoreR.string.select_from_storage),
                            style = MaterialTheme.typography.bodyLarge,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                        val fileName = remember(localFileUri) {
                            localFileUri?.let { runCatching { it.displayName }.getOrNull() }
                        }
                        if (!fileName.isNullOrEmpty()) {
                            Text(
                                text = fileName,
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                }

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .selectable(
                            selected = source == PatchSource.URL,
                            onClick = { source = PatchSource.URL },
                            role = Role.RadioButton
                        )
                        .padding(vertical = 10.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = source == PatchSource.URL,
                        onClick = null
                    )
                    Spacer(Modifier.width(16.dp))
                    Text(
                        text = stringResource(CoreR.string.download_dialog_title),
                        style = MaterialTheme.typography.bodyLarge,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                }

                if (source == PatchSource.URL) {
                    OutlinedTextField(
                        value = url,
                        onValueChange = {
                            url = it
                            isUrlError = false
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(top = 4.dp),
                        shape = RoundedCornerShape(12.dp),
                        label = { Text(stringResource(CoreR.string.download_dialog_msg)) },
                        isError = isUrlError,
                        supportingText = if (isUrlError) {
                            { Text(stringResource(CoreR.string.download_dialog_title)) }
                        } else null,
                        singleLine = true,
                        keyboardOptions = KeyboardOptions(
                            keyboardType = KeyboardType.Uri,
                            imeAction = ImeAction.Done
                        ),
                        keyboardActions = KeyboardActions(
                            onDone = { submit() }
                        )
                    )
                }
            }

            if (showUsbDestination) {
                HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))

                Text(
                    text = stringResource(CoreR.string.output_destination),
                    style = MaterialTheme.typography.titleSmall,
                    color = MaterialTheme.colorScheme.primary
                )

                Column(modifier = Modifier.selectableGroup()) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(12.dp))
                            .selectable(
                                selected = selectedUsb == null,
                                onClick = {
                                    preferFile = true
                                    selectedUsb = null
                                },
                                role = Role.RadioButton
                            )
                            .padding(vertical = 10.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = selectedUsb == null,
                            onClick = null
                        )
                        Spacer(Modifier.width(16.dp))
                        Text(
                            text = stringResource(CoreR.string.save_to_file),
                            style = MaterialTheme.typography.bodyLarge,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                    }

                    for (device in usbDevices) {
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .clip(RoundedCornerShape(12.dp))
                                .selectable(
                                    selected = selectedUsb == device.deviceName,
                                    onClick = {
                                        preferFile = false
                                        selectedUsb = device.deviceName
                                    },
                                    role = Role.RadioButton
                                )
                                .padding(vertical = 10.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(
                                selected = selectedUsb == device.deviceName,
                                onClick = null
                            )
                            Spacer(Modifier.width(16.dp))
                            Text(
                                text = "USB: ${device.label}",
                                style = MaterialTheme.typography.bodyLarge,
                                color = MaterialTheme.colorScheme.onSurface
                            )
                        }
                    }
                }

                if (usbDevices.isEmpty()) {
                    Text(
                        text = stringResource(CoreR.string.no_usb_drive_found),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                } else if (selectedUsb != null) {
                    Text(
                        text = stringResource(CoreR.string.usb_overwrite_warning),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.error
                    )
                }
            }
        }
    }
}

