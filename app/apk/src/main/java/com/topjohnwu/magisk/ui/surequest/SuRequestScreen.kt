package com.topjohnwu.magisk.ui.surequest

import android.view.MotionEvent
import android.widget.Toast
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringArrayResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.superuser.SharedUidBadge
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalComposeUiApi::class)
@Composable
fun SuRequestScreen(
    viewModel: SuRequestViewModel,
    modifier: Modifier = Modifier
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    if (!uiState.showUi) return

    val context = LocalContext.current
    val icon = uiState.icon
    val title = uiState.title
    val packageName = uiState.packageName
    val grantEnabled = uiState.grantEnabled
    val denyCountdown = uiState.denyCountdown
    val selectedPosition = uiState.selectedItemPosition
    val timeoutEntries = stringArrayResource(CoreR.array.allow_timeout).toList()
    // Slider order: Once(1), 10min(2), 20min(3), 30min(4), 60min(5), Forever(0)
    val sliderToIndex = intArrayOf(1, 2, 3, 4, 5, 0)
    val indexToSlider = remember {
        IntArray(sliderToIndex.size).also { arr ->
            sliderToIndex.forEachIndexed { slider, orig -> arr[orig] = slider }
        }
    }
    val sliderValue = indexToSlider[selectedPosition].toFloat()
    val sliderLabel by remember(sliderValue) {
        derivedStateOf { timeoutEntries[sliderToIndex[sliderValue.toInt()]] }
    }

    val denyText = if (denyCountdown > 0) {
        "${stringResource(CoreR.string.deny)} ($denyCountdown)"
    } else {
        stringResource(CoreR.string.deny)
    }

    Box(
        modifier = modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Card(
            modifier = Modifier
                .padding(horizontal = 16.dp, vertical = 16.dp)
                .fillMaxWidth()
                .widthIn(max = 560.dp),
            shape = RoundedCornerShape(28.dp),
            colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
        ) {
            Column(
                modifier = Modifier
                    .verticalScroll(rememberScrollState())
                    .padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 4.dp)
                ) {
                    if (icon != null) {
                        Image(
                            painter = rememberDrawablePainter(icon),
                            contentDescription = null,
                            modifier = Modifier.size(44.dp)
                        )
                        Spacer(Modifier.width(14.dp))
                    }
                    Text(
                        text = title,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis,
                        modifier = Modifier.weight(1f, fill = false),
                    )
                    if (uiState.isSharedUid) {
                        Spacer(Modifier.width(6.dp))
                        SharedUidBadge()
                    }
                }

                Spacer(Modifier.height(8.dp))

                Text(
                    text = packageName,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 4.dp),
                )

                Spacer(Modifier.height(16.dp))

                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .background(MaterialTheme.colorScheme.errorContainer)
                        .padding(12.dp)
                ) {
                    Text(
                        text = stringResource(CoreR.string.su_warning),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onErrorContainer,
                        fontWeight = FontWeight.Medium,
                        textAlign = TextAlign.Center,
                        modifier = Modifier.fillMaxWidth()
                    )
                }

                Spacer(Modifier.height(16.dp))

                Text(
                    text = "${stringResource(CoreR.string.request_timeout)}: $sliderLabel",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.fillMaxWidth().padding(start = 4.dp),
                )
                Spacer(Modifier.height(8.dp))
                Slider(
                    value = sliderValue,
                    onValueChange = { value ->
                        viewModel.spinnerTouched()
                        val pos = value.toInt().coerceIn(0, sliderToIndex.lastIndex)
                        viewModel.setSelectedItemPosition(sliderToIndex[pos])
                    },
                    valueRange = 0f..5f,
                    steps = 4,
                    modifier = Modifier.fillMaxWidth()
                )
                Spacer(Modifier.height(20.dp))

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(10.dp)
                ) {
                    OutlinedButton(
                        onClick = { viewModel.denyPressed() },
                        shape = RoundedCornerShape(20.dp),
                        contentPadding = PaddingValues(vertical = 12.dp),
                        modifier = Modifier.weight(1f),
                    ) {
                        Text(
                            text = denyText,
                            style = MaterialTheme.typography.labelLarge,
                        )
                    }
                    Button(
                        enabled = grantEnabled,
                        onClick = { viewModel.grantPressed() },
                        shape = RoundedCornerShape(20.dp),
                        contentPadding = PaddingValues(vertical = 12.dp),
                        modifier = Modifier
                            .weight(1f)
                            .then(
                                if (uiState.useTapjackProtection) {
                                    Modifier.pointerInteropFilter { event ->
                                        if (event.flags and MotionEvent.FLAG_WINDOW_IS_OBSCURED != 0 ||
                                            event.flags and MotionEvent.FLAG_WINDOW_IS_PARTIALLY_OBSCURED != 0
                                        ) {
                                            if (event.action == MotionEvent.ACTION_UP) {
                                                context.toast(
                                                    CoreR.string.touch_filtered_warning,
                                                    Toast.LENGTH_SHORT
                                                )
                                            }
                                            true
                                        } else {
                                            false
                                        }
                                    }
                                } else Modifier
                            )
                    ) {
                        Text(
                            text = stringResource(CoreR.string.grant),
                            style = MaterialTheme.typography.labelLarge,
                        )
                    }
                }
            }
        }
    }
}
