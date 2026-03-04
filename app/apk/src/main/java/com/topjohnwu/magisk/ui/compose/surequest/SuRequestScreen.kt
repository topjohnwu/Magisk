package com.topjohnwu.magisk.ui.compose.surequest

import android.graphics.drawable.Drawable
import android.view.MotionEvent
import android.widget.Toast
import androidx.compose.animation.*
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.graphics.drawable.toBitmap
import androidx.databinding.Observable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.R as CoreR
import com.topjohnwu.magisk.ui.compose.theme.magiskComposeColorScheme
import com.topjohnwu.magisk.ui.surequest.SuRequestViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuRequestScreen(
    viewModel: SuRequestViewModel,
    showContent: Boolean,
    onTimeoutSelected: (Int) -> Unit,
    onSpinnerTouched: () -> Unit,
    onGrant: () -> Unit,
    onDeny: () -> Unit
) {
    val context = LocalContext.current
    val resources = context.resources
    val timeoutItems = remember { resources.getStringArray(CoreR.array.allow_timeout).toList() }
    val denyDefaultLabel = stringResource(id = CoreR.string.deny)

    var selectedTimeout by remember { mutableStateOf(viewModel.selectedItemPosition) }
    var grantEnabled by remember { mutableStateOf(viewModel.grantEnabled) }
    var denyLabel by remember { mutableStateOf(viewModel.denyText.getText(resources).toString()) }

    DisposableEffect(viewModel) {
        val callback = object : Observable.OnPropertyChangedCallback() {
            override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
                when (propertyId) {
                    BR.selectedItemPosition -> selectedTimeout = viewModel.selectedItemPosition
                    BR.grantEnabled -> grantEnabled = viewModel.grantEnabled
                    BR.denyText -> denyLabel = viewModel.denyText.getText(resources).toString()
                }
            }
        }
        viewModel.addOnPropertyChangedCallback(callback)
        onDispose { viewModel.removeOnPropertyChangedCallback(callback) }
    }

    val grantTouchFilter: (MotionEvent) -> Boolean = remember {
        { event ->
            val obscured = (event.flags and MotionEvent.FLAG_WINDOW_IS_OBSCURED != 0) ||
                    (event.flags and MotionEvent.FLAG_WINDOW_IS_PARTIALLY_OBSCURED != 0)
            if (obscured && event.action == MotionEvent.ACTION_UP) {
                Toast.makeText(context, CoreR.string.touch_filtered_warning, Toast.LENGTH_SHORT).show()
            }
            obscured && Config.suTapjack
        }
    }

    val iconPainter = remember(showContent) {
        runCatching {
            viewModel.packageName
            viewModel.title
            viewModel.safePainter()
        }.getOrNull()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black.copy(alpha = 0.62f))
            .statusBarsPadding()
            .navigationBarsPadding(),
        contentAlignment = Alignment.Center
    ) {
        if (!showContent) {
            CircularProgressIndicator(color = Color.White)
            return@Box
        }

        var dropdownExpanded by remember { mutableStateOf(false) }

        Surface(
            modifier = Modifier
                .padding(horizontal = 24.dp, vertical = 20.dp)
                .fillMaxWidth()
                .widthIn(max = 420.dp),
            shape = RoundedCornerShape(24.dp),
            color = MaterialTheme.colorScheme.surface,
            tonalElevation = 8.dp,
            shadowElevation = 12.dp
        ) {
            Column(
                modifier = Modifier.padding(24.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Text(
                    text = stringResource(id = CoreR.string.su_request_title),
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    textAlign = TextAlign.Center,
                    modifier = Modifier.fillMaxWidth()
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Surface(
                        modifier = Modifier.size(48.dp),
                        shape = RoundedCornerShape(12.dp),
                        color = MaterialTheme.colorScheme.surfaceVariant
                    ) {
                        Box(contentAlignment = Alignment.Center) {
                            if (iconPainter != null) {
                                Image(
                                    painter = iconPainter,
                                    contentDescription = null,
                                    modifier = Modifier
                                        .size(36.dp)
                                        .clip(RoundedCornerShape(6.dp))
                                )
                            } else {
                                Icon(
                                    Icons.Rounded.Security,
                                    contentDescription = null,
                                    modifier = Modifier.size(24.dp),
                                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                                )
                            }
                        }
                    }
                    Spacer(Modifier.width(12.dp))
                    Column(
                        modifier = Modifier.weight(1f),
                        verticalArrangement = Arrangement.Center
                    ) {
                        Text(
                            text = viewModel.title,
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        Text(
                            text = viewModel.packageName,
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                }

                Surface(
                    onClick = {
                        if (grantEnabled) {
                            onSpinnerTouched()
                            dropdownExpanded = true
                        }
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp),
                    shape = RoundedCornerShape(12.dp),
                    color = MaterialTheme.colorScheme.surfaceContainerHigh,
                    enabled = grantEnabled
                ) {
                    Row(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(horizontal = 14.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                Icons.Rounded.Timer,
                                contentDescription = null,
                                modifier = Modifier.size(18.dp),
                                tint = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                            Spacer(Modifier.width(10.dp))
                            Text(
                                text = timeoutItems.getOrNull(selectedTimeout).orEmpty(),
                                style = MaterialTheme.typography.bodyMedium,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                        Icon(
                            Icons.Rounded.ExpandMore,
                            contentDescription = null,
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }

                DropdownMenu(
                    expanded = dropdownExpanded,
                    onDismissRequest = { dropdownExpanded = false },
                    modifier = Modifier.background(MaterialTheme.colorScheme.surface),
                    shape = RoundedCornerShape(12.dp)
                ) {
                    timeoutItems.forEachIndexed { index, item ->
                        DropdownMenuItem(
                            text = {
                                Text(
                                    text = item,
                                    fontWeight = if (index == selectedTimeout) FontWeight.SemiBold else FontWeight.Normal
                                )
                            },
                            onClick = {
                                dropdownExpanded = false
                                selectedTimeout = index
                                onTimeoutSelected(index)
                            },
                            leadingIcon = {
                                if (index == selectedTimeout) {
                                    Icon(
                                        Icons.Rounded.Check,
                                        contentDescription = null,
                                        tint = MaterialTheme.colorScheme.primary
                                    )
                                }
                            }
                        )
                    }
                }

                Text(
                    text = stringResource(id = CoreR.string.su_warning),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.error,
                    fontWeight = FontWeight.SemiBold,
                    textAlign = TextAlign.Center,
                    modifier = Modifier.fillMaxWidth()
                )

                HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.6f))

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    TextButton(
                        onClick = onDeny,
                        modifier = Modifier
                            .weight(1f)
                            .heightIn(min = 44.dp)
                    ) {
                        Text(
                            text = denyLabel.ifBlank { denyDefaultLabel },
                            maxLines = 1,
                            softWrap = false,
                            overflow = TextOverflow.Ellipsis,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    TextButton(
                        onClick = onGrant,
                        enabled = grantEnabled,
                        modifier = Modifier
                            .weight(1f)
                            .heightIn(min = 44.dp)
                            .pointerInteropFilter { grantTouchFilter(it) }
                    ) {
                        Text(
                            text = stringResource(id = CoreR.string.grant),
                            maxLines = 1,
                            softWrap = false,
                            overflow = TextOverflow.Ellipsis,
                            fontWeight = FontWeight.SemiBold
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun suRequestColorScheme(useDynamicColor: Boolean, darkTheme: Boolean) =
    magiskComposeColorScheme(
        useDynamicColor = useDynamicColor,
        darkTheme = darkTheme
    )

private fun SuRequestViewModel.safePainter(): Painter? {
    val drawable: Drawable = runCatching { icon }.getOrNull() ?: return null
    val bitmap = runCatching { drawable.toBitmap() }.getOrNull()?.asImageBitmap() ?: return null
    return BitmapPainter(bitmap)
}
