package com.topjohnwu.magisk.ui.compose.surequest

import android.graphics.drawable.Drawable
import android.os.Build
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
import com.topjohnwu.magisk.ui.compose.fallbackColorScheme
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
            .background(Color.Black.copy(alpha = 0.6f))
    ) {
        if (!showContent) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center), color = Color.White)
        } else {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .statusBarsPadding()
                    .navigationBarsPadding(),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Spacer(Modifier.weight(0.15f))

                // 1. Header & Content Card
                Surface(
                    modifier = Modifier
                        .padding(horizontal = 24.dp)
                        .fillMaxWidth()
                        .widthIn(max = 400.dp),
                    shape = RoundedCornerShape(40.dp),
                    color = MaterialTheme.colorScheme.surface,
                    tonalElevation = 8.dp,
                    shadowElevation = 12.dp
                ) {
                    Column(
                        modifier = Modifier.padding(32.dp),
                        horizontalAlignment = Alignment.CenterHorizontally,
                        verticalArrangement = Arrangement.spacedBy(20.dp)
                    ) {
                        // App Identity Section
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Surface(
                                modifier = Modifier.size(90.dp),
                                shape = RoundedCornerShape(28.dp),
                                color = MaterialTheme.colorScheme.primaryContainer,
                                tonalElevation = 2.dp
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    if (iconPainter != null) {
                                        Image(
                                            painter = iconPainter,
                                            contentDescription = null,
                                            modifier = Modifier
                                                .size(56.dp)
                                                .clip(RoundedCornerShape(8.dp))
                                        )
                                    } else {
                                        Icon(
                                            Icons.Rounded.Security,
                                            null,
                                            modifier = Modifier.size(44.dp),
                                            tint = MaterialTheme.colorScheme.onPrimaryContainer
                                        )
                                    }
                                }
                            }
                            Spacer(Modifier.height(16.dp))
                            Text(
                                text = stringResource(id = CoreR.string.su_request_title).uppercase(),
                                style = MaterialTheme.typography.labelLarge,
                                fontWeight = FontWeight.Black,
                                color = MaterialTheme.colorScheme.primary,
                                letterSpacing = 1.5.sp
                            )
                            Spacer(Modifier.height(8.dp))
                            Text(
                                text = viewModel.title,
                                style = MaterialTheme.typography.headlineSmall,
                                fontWeight = FontWeight.ExtraBold,
                                textAlign = TextAlign.Center
                            )
                            Text(
                                text = viewModel.packageName,
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                modifier = Modifier.alpha(0.6f),
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }

                        // Security Disclaimer
                        Surface(
                            color = MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.2f),
                            shape = RoundedCornerShape(16.dp),
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Row(
                                modifier = Modifier.padding(16.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Icon(
                                    Icons.Rounded.GppMaybe,
                                    null,
                                    tint = MaterialTheme.colorScheme.error,
                                    modifier = Modifier.size(24.dp)
                                )
                                Spacer(Modifier.width(12.dp))
                                Text(
                                    text = stringResource(id = CoreR.string.su_warning),
                                    style = MaterialTheme.typography.bodySmall,
                                    color = MaterialTheme.colorScheme.onErrorContainer,
                                    fontWeight = FontWeight.Bold
                                )
                            }
                        }

                        // Timer Dropdown
                        var dropdownExpanded by remember { mutableStateOf(false) }
                        Surface(
                            onClick = { if (grantEnabled) { dropdownExpanded = true; onSpinnerTouched() } },
                            modifier = Modifier
                                .fillMaxWidth()
                                .height(56.dp),
                            shape = RoundedCornerShape(16.dp),
                            color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f),
                            enabled = grantEnabled
                        ) {
                            Row(
                                modifier = Modifier.padding(horizontal = 20.dp),
                                verticalAlignment = Alignment.CenterVertically,
                                horizontalArrangement = Arrangement.SpaceBetween
                            ) {
                                Row(verticalAlignment = Alignment.CenterVertically) {
                                    Icon(
                                        Icons.Rounded.Timer,
                                        null,
                                        tint = MaterialTheme.colorScheme.onSurfaceVariant,
                                        modifier = Modifier.size(20.dp)
                                    )
                                    Spacer(Modifier.width(12.dp))
                                    Text(
                                        text = timeoutItems.getOrNull(selectedTimeout) ?: "",
                                        style = MaterialTheme.typography.bodyLarge,
                                        fontWeight = FontWeight.Bold
                                    )
                                }
                                Icon(Icons.Rounded.UnfoldMore, null, tint = MaterialTheme.colorScheme.outline)
                            }
                            DropdownMenu(
                                expanded = dropdownExpanded,
                                onDismissRequest = { dropdownExpanded = false },
                                modifier = Modifier.background(MaterialTheme.colorScheme.surfaceContainerHighest),
                                shape = RoundedCornerShape(16.dp)
                            ) {
                                timeoutItems.forEachIndexed { index, item ->
                                    DropdownMenuItem(
                                        text = { 
                                            Text(
                                                item, 
                                                fontWeight = if (index == selectedTimeout) FontWeight.Black else FontWeight.Medium
                                            ) 
                                        },
                                        onClick = {
                                            dropdownExpanded = false
                                            selectedTimeout = index
                                            onTimeoutSelected(index)
                                        },
                                        leadingIcon = {
                                            if (index == selectedTimeout) {
                                                Icon(Icons.Rounded.Check, null, tint = MaterialTheme.colorScheme.primary)
                                            }
                                        }
                                    )
                                }
                            }
                        }
                    }
                }

                Spacer(Modifier.weight(1f))

                // 2. Bottom Floating Action Bar (Glass style)
                Surface(
                    modifier = Modifier
                        .padding(24.dp)
                        .fillMaxWidth()
                        .widthIn(max = 400.dp)
                        .height(88.dp),
                    shape = RoundedCornerShape(32.dp),
                    color = MaterialTheme.colorScheme.surfaceContainerHighest.copy(alpha = 0.88f),
                    tonalElevation = 12.dp,
                    shadowElevation = 16.dp
                ) {
                    Row(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(horizontal = 16.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        OutlinedButton(
                            onClick = onDeny,
                            modifier = Modifier
                                .weight(1f)
                                .height(56.dp),
                            shape = RoundedCornerShape(20.dp),
                            border = androidx.compose.foundation.BorderStroke(
                                1.dp,
                                MaterialTheme.colorScheme.outline.copy(alpha = 0.3f)
                            )
                        ) {
                            Text(
                                denyLabel.uppercase(),
                                fontWeight = FontWeight.Black,
                                color = MaterialTheme.colorScheme.error
                            )
                        }

                        Button(
                            onClick = onGrant,
                            enabled = grantEnabled,
                            modifier = Modifier
                                .weight(1f)
                                .height(56.dp)
                                .pointerInteropFilter { grantTouchFilter(it) },
                            shape = RoundedCornerShape(20.dp),
                            colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)
                        ) {
                            Icon(Icons.Rounded.Shield, null, modifier = Modifier.size(18.dp))
                            Spacer(Modifier.width(8.dp))
                            Text(
                                stringResource(id = CoreR.string.grant).uppercase(),
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
fun suRequestColorScheme(useDynamicColor: Boolean, darkTheme: Boolean) = when {
    useDynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
        if (darkTheme) androidx.compose.material3.dynamicDarkColorScheme(LocalContext.current)
        else androidx.compose.material3.dynamicLightColorScheme(LocalContext.current)
    }
    else -> fallbackColorScheme(darkTheme)
}

private fun SuRequestViewModel.safePainter(): Painter? {
    val drawable: Drawable = runCatching { icon }.getOrNull() ?: return null
    val bitmap = runCatching { drawable.toBitmap() }.getOrNull()?.asImageBitmap() ?: return null
    return BitmapPainter(bitmap)
}
