package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.os.Build
import android.widget.TextView
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts.GetContent
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
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.flow.collect
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun InstallScreen(
    onStartFlash: (action: String, uri: Uri?) -> Unit
) {
    val viewModel: InstallViewModel = viewModel(factory = VMFactory)
    val state by viewModel.uiState.collectAsState()

    val patchPicker = rememberLauncherForActivityResult(GetContent()) { uri ->
        uri?.let { viewModel.onPatchFileSelected(it) }
    }

    val secondSlotDialog = rememberConfirmDialog()
    val secondSlotTitle = stringResource(android.R.string.dialog_alert_title)
    val secondSlotMsg = stringResource(CoreR.string.install_inactive_slot_msg)

    LaunchedEffect(state.requestFilePicker) {
        if (state.requestFilePicker) {
            patchPicker.launch("*/*")
            viewModel.onFilePickerConsumed()
        }
    }

    LaunchedEffect(state.showSecondSlotWarning) {
        if (state.showSecondSlotWarning) {
            val result = secondSlotDialog.awaitConfirm(title = secondSlotTitle, content = secondSlotMsg)
            viewModel.onSecondSlotWarningConsumed()
            if (result == ConfirmResult.Confirmed) {
                viewModel.install()
            }
        }
    }

    LaunchedEffect(viewModel) {
        viewModel.navEvents.collect { route ->
            if (route is Route.Flash) {
                onStartFlash(route.action, route.additionalData?.let(Uri::parse))
            }
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 140.dp, start = 20.dp, end = 20.dp, top = 12.dp),
            verticalArrangement = Arrangement.spacedBy(28.dp)
        ) {
            if (!viewModel.skipOptions) {
                item {
                    InstallSection(
                        title = stringResource(id = CoreR.string.install_options_title),
                        icon = Icons.Rounded.Tune
                    ) {
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.keep_dm_verity),
                            checked = Config.keepVerity,
                            visible = !Info.isSAR,
                            onToggle = { Config.keepVerity = it }
                        )
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.keep_force_encryption),
                            checked = Config.keepEnc,
                            visible = Info.isFDE,
                            onToggle = { Config.keepEnc = it }
                        )
                        ExpressiveToggleRow(
                            title = stringResource(id = CoreR.string.recovery_mode),
                            checked = Config.recovery,
                            visible = !Info.ramdisk,
                            onToggle = { Config.recovery = it }
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
                                    subtitle = if (state.patchUri != null && state.method == InstallViewModel.Method.PATCH) {
                                        state.patchUri?.displayName
                                            ?: stringResource(id = CoreR.string.install_select_patch_file_subtitle)
                                    } else {
                                        stringResource(id = CoreR.string.install_select_patch_file_subtitle)
                                    },
                                    selected = state.method == InstallViewModel.Method.PATCH,
                                    icon = Icons.Rounded.FileCopy,
                                    onClick = {
                                        viewModel.selectMethod(InstallViewModel.Method.PATCH)
                                    }
                                )
                                if (viewModel.isRooted) {
                                    ExpressiveMethodRow(
                                        title = stringResource(id = CoreR.string.direct_install),
                                        subtitle = stringResource(id = CoreR.string.install_direct_install_subtitle),
                                        selected = state.method == InstallViewModel.Method.DIRECT,
                                        icon = Icons.Rounded.FlashOn,
                                        onClick = {
                                            viewModel.selectMethod(InstallViewModel.Method.DIRECT)
                                            viewModel.install()
                                        }
                                    )
                                }
                                if (!viewModel.noSecondSlot) {
                                    ExpressiveMethodRow(
                                        title = stringResource(id = CoreR.string.install_inactive_slot),
                                        subtitle = stringResource(id = CoreR.string.install_inactive_slot_subtitle),
                                        selected = state.method == InstallViewModel.Method.INACTIVE_SLOT,
                                        icon = Icons.Rounded.DynamicFeed,
                                        onClick = {
                                            viewModel.selectMethod(InstallViewModel.Method.INACTIVE_SLOT)
                                        }
                                    )
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
