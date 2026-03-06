package com.topjohnwu.magisk.ui.surequest

import android.graphics.drawable.Drawable
import android.os.Build
import android.view.MotionEvent
import android.widget.Toast
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Check
import androidx.compose.material.icons.rounded.Security
import androidx.compose.material.icons.rounded.Shield
import androidx.compose.material.icons.rounded.Timer
import androidx.compose.material.icons.rounded.UnfoldMore
import androidx.compose.material.icons.rounded.Warning
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.painter.BitmapPainter
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.graphics.drawable.toBitmap
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ui.theme.magiskComposeColorScheme
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuRequestScreen(
    viewModel: SuRequestViewModel,
    showContent: Boolean = viewModel.showUi,
    onTimeoutSelected: (Int) -> Unit = { viewModel.selectedItemPosition = it },
    onSpinnerTouched: () -> Unit = { viewModel.spinnerTouched() },
    onGrant: () -> Unit = { viewModel.grantPressed() },
    onDeny: () -> Unit = { viewModel.denyPressed() }
) {
    val context = LocalContext.current
    val resources = context.resources
    val timeoutItems = remember { resources.getStringArray(CoreR.array.allow_timeout).toList() }
    val selectedTimeout = viewModel.selectedItemPosition
    val grantEnabled = viewModel.grantEnabled
    val denyLabel = if (viewModel.denyCountdown > 0) {
        "${stringResource(CoreR.string.deny)} (${viewModel.denyCountdown})"
    } else {
        stringResource(CoreR.string.deny)
    }

    val grantTouchFilter: (MotionEvent) -> Boolean = remember {
        { event ->
            val partiallyObscured = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                event.flags and MotionEvent.FLAG_WINDOW_IS_PARTIALLY_OBSCURED != 0
            } else {
                false
            }
            val obscured = (event.flags and MotionEvent.FLAG_WINDOW_IS_OBSCURED != 0) ||
                partiallyObscured
            if (obscured && event.action == MotionEvent.ACTION_UP) {
                Toast.makeText(context, CoreR.string.touch_filtered_warning, Toast.LENGTH_SHORT)
                    .show()
            }
            obscured && Config.suTapjack
        }
    }

    val iconPainter = remember(showContent) {
        runCatching {
            viewModel.safePainter()
        }.getOrNull()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Transparent),
        contentAlignment = Alignment.Center
    ) {
        if (!showContent) {
            CircularProgressIndicator(strokeCap = StrokeCap.Round)
            return@Box
        }

        var dropdownExpanded by remember { mutableStateOf(false) }

        ElevatedCard(
            modifier = Modifier
                .padding(horizontal = 20.dp)
                .fillMaxWidth(),
            shape = RoundedCornerShape(
                topStart = 48.dp,
                bottomEnd = 48.dp,
                topEnd = 16.dp,
                bottomStart = 16.dp
            ),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surface),
            elevation = CardDefaults.elevatedCardElevation(defaultElevation = 12.dp)
        ) {
            Box {
                Icon(
                    painter = painterResource(id = CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier
                        .size(180.dp)
                        .align(Alignment.TopEnd)
                        .offset(x = 40.dp, y = (-20).dp)
                        .alpha(0.05f),
                    tint = MaterialTheme.colorScheme.primary
                )

                Column(
                    modifier = Modifier.padding(28.dp),
                    verticalArrangement = Arrangement.spacedBy(20.dp)
                ) {
                    // Header Row with shield icon
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Surface(
                            color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f),
                            shape = RoundedCornerShape(12.dp),
                            modifier = Modifier.size(32.dp)
                        ) {
                            Icon(
                                Icons.Rounded.Security, null,
                                modifier = Modifier.padding(6.dp),
                                tint = MaterialTheme.colorScheme.onPrimaryContainer
                            )
                        }
                        Spacer(Modifier.width(16.dp))
                        Text(
                            text = stringResource(id = CoreR.string.su_request_title).uppercase(),
                            style = MaterialTheme.typography.labelLarge,
                            fontWeight = FontWeight.Black,
                            letterSpacing = 1.2.sp,
                            color = MaterialTheme.colorScheme.outline
                        )
                    }

                    // Main App Info Card
                    Surface(
                        color = MaterialTheme.colorScheme.surfaceContainerHigh,
                        shape = RoundedCornerShape(20.dp),
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Row(
                            modifier = Modifier.padding(16.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Surface(
                                modifier = Modifier.size(64.dp),
                                shape = RoundedCornerShape(16.dp),
                                color = MaterialTheme.colorScheme.surface,
                                tonalElevation = 4.dp
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    if (iconPainter != null) {
                                        Image(
                                            painter = iconPainter,
                                            contentDescription = null,
                                            modifier = Modifier.size(44.dp)
                                        )
                                    } else {
                                        Icon(
                                            Icons.Rounded.Shield,
                                            null,
                                            modifier = Modifier.size(32.dp),
                                            tint = MaterialTheme.colorScheme.primary
                                        )
                                    }
                                }
                            }
                            Spacer(Modifier.width(20.dp))
                            Column(modifier = Modifier.weight(1f)) {
                                Text(
                                    text = viewModel.title,
                                    style = MaterialTheme.typography.headlineSmall,
                                    fontWeight = FontWeight.Black,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                                Text(
                                    text = viewModel.packageName,
                                    style = MaterialTheme.typography.bodySmall,
                                    color = MaterialTheme.colorScheme.primary,
                                    fontWeight = FontWeight.Bold,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            }
                        }
                    }

                    // Interactive Section
                    Column(
                        verticalArrangement = Arrangement.spacedBy(12.dp),
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        // Timeout Selector (Custom Dropdown)
                        Box {
                            Surface(
                                onClick = {
                                    if (grantEnabled) {
                                        onSpinnerTouched()
                                        dropdownExpanded = true
                                    }
                                },
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .height(56.dp),
                                shape = RoundedCornerShape(16.dp),
                                color = MaterialTheme.colorScheme.surfaceContainerHighest,
                                enabled = grantEnabled
                            ) {
                                Row(
                                    modifier = Modifier
                                        .fillMaxSize()
                                        .padding(horizontal = 16.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Icon(
                                        Icons.Rounded.Timer,
                                        null,
                                        modifier = Modifier.size(20.dp),
                                        tint = MaterialTheme.colorScheme.primary
                                    )
                                    Spacer(Modifier.width(12.dp))
                                    Text(
                                        text = timeoutItems.getOrNull(selectedTimeout).orEmpty(),
                                        style = MaterialTheme.typography.bodyMedium,
                                        fontWeight = FontWeight.Bold,
                                        modifier = Modifier.weight(1f)
                                    )
                                    Icon(
                                        Icons.Rounded.UnfoldMore,
                                        null,
                                        modifier = Modifier.size(20.dp),
                                        tint = MaterialTheme.colorScheme.outline
                                    )
                                }
                            }

                            DropdownMenu(
                                expanded = dropdownExpanded,
                                onDismissRequest = { dropdownExpanded = false },
                                modifier = Modifier.background(MaterialTheme.colorScheme.surfaceContainerHigh),
                                shape = RoundedCornerShape(16.dp)
                            ) {
                                timeoutItems.forEachIndexed { index, item ->
                                    DropdownMenuItem(
                                        text = {
                                            Text(
                                                text = item,
                                                fontWeight = if (index == selectedTimeout) FontWeight.Black else FontWeight.Medium
                                            )
                                        },
                                        onClick = {
                                            dropdownExpanded = false
                                            onTimeoutSelected(index)
                                        },
                                        leadingIcon = {
                                            if (index == selectedTimeout) {
                                                Icon(
                                                    Icons.Rounded.Check,
                                                    null,
                                                    tint = MaterialTheme.colorScheme.primary
                                                )
                                            }
                                        }
                                    )
                                }
                            }
                        }

                        // Warning Text Section
                        Surface(
                            color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.15f),
                            shape = RoundedCornerShape(14.dp),
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Row(
                                modifier = Modifier.padding(14.dp),
                                verticalAlignment = Alignment.CenterVertically,
                                horizontalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                Icon(
                                    Icons.Rounded.Warning,
                                    null,
                                    modifier = Modifier.size(20.dp),
                                    tint = MaterialTheme.colorScheme.error
                                )
                                Text(
                                    text = stringResource(id = CoreR.string.su_warning),
                                    style = MaterialTheme.typography.bodySmall,
                                    color = MaterialTheme.colorScheme.error,
                                    fontWeight = FontWeight.Bold,
                                    lineHeight = 16.sp,
                                    modifier = Modifier.weight(1f)
                                )
                            }
                        }
                    }

                    Spacer(Modifier.height(4.dp))

                    // Buttons Action Row
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        Surface(
                            onClick = onDeny,
                            modifier = Modifier
                                .weight(1f)
                                .height(60.dp),
                            shape = RoundedCornerShape(20.dp),
                            color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.25f)
                        ) {
                            Box(contentAlignment = Alignment.Center) {
                                Text(
                                    text = denyLabel.uppercase(),
                                    style = MaterialTheme.typography.titleSmall,
                                    fontWeight = FontWeight.Black,
                                    color = MaterialTheme.colorScheme.error
                                )
                            }
                        }

                        Button(
                            onClick = onGrant,
                            enabled = grantEnabled,
                            modifier = Modifier
                                .weight(1f)
                                .height(60.dp)
                                .pointerInteropFilter { grantTouchFilter(it) },
                            shape = RoundedCornerShape(20.dp),
                            colors = ButtonDefaults.buttonColors(
                                containerColor = MaterialTheme.colorScheme.primary,
                                contentColor = MaterialTheme.colorScheme.onPrimary
                            ),
                            elevation = ButtonDefaults.buttonElevation(defaultElevation = 2.dp)
                        ) {
                            Text(
                                text = stringResource(id = CoreR.string.grant).uppercase(),
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.Black
                            )
                        }
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
