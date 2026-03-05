package com.topjohnwu.magisk.ui.surequest

import android.view.MotionEvent
import android.widget.Toast
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringArrayResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.ui.superuser.SharedUidBadge
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.R as CoreR
import com.topjohnwu.magisk.ui.util.rememberDrawablePainter
import top.yukonga.miuix.kmp.basic.ButtonDefaults
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.extra.SuperDropdown
import top.yukonga.miuix.kmp.theme.MiuixTheme

@OptIn(ExperimentalComposeUiApi::class)
@Composable
fun SuRequestScreen(viewModel: SuRequestViewModel) {
    if (!viewModel.showUi) return

    val context = LocalContext.current
    val icon = viewModel.icon
    val title = viewModel.title
    val packageName = viewModel.packageName
    val grantEnabled = viewModel.grantEnabled
    val denyCountdown = viewModel.denyCountdown
    val selectedPosition = viewModel.selectedItemPosition
    val timeoutEntries = stringArrayResource(CoreR.array.allow_timeout).toList()

    val denyText = if (denyCountdown > 0) {
        "${stringResource(CoreR.string.deny)} ($denyCountdown)"
    } else {
        stringResource(CoreR.string.deny)
    }

    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Card(
            modifier = Modifier
                .widthIn(min = 320.dp, max = 420.dp)
                .padding(24.dp)
        ) {
            Column(
                modifier = Modifier.padding(20.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.padding(horizontal = 8.dp)
                ) {
                    if (icon != null) {
                        Image(
                            painter = rememberDrawablePainter(icon),
                            contentDescription = null,
                            modifier = Modifier.size(48.dp)
                        )
                        Spacer(Modifier.width(12.dp))
                    }
                    Column {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Text(
                                text = title,
                                style = MiuixTheme.textStyles.body1,
                                fontWeight = FontWeight.Bold,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis,
                                modifier = Modifier.weight(1f, fill = false),
                            )
                            if (viewModel.isSharedUid) {
                                Spacer(Modifier.width(6.dp))
                                SharedUidBadge()
                            }
                        }
                        Text(
                            text = packageName,
                            style = MiuixTheme.textStyles.body2,
                            color = MiuixTheme.colorScheme.onSurfaceVariantSummary,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis,
                        )
                    }
                }

                Spacer(Modifier.height(16.dp))

                Text(
                    text = stringResource(CoreR.string.su_request_title),
                    style = MiuixTheme.textStyles.body2,
                    color = MiuixTheme.colorScheme.primary,
                    fontWeight = FontWeight.Bold,
                    textAlign = TextAlign.Center,
                )

                Spacer(Modifier.height(8.dp))

                SuperDropdown(
                    title = stringResource(CoreR.string.request_timeout),
                    items = timeoutEntries,
                    selectedIndex = selectedPosition,
                    onSelectedIndexChange = { index ->
                        viewModel.spinnerTouched()
                        viewModel.selectedItemPosition = index
                    },
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(Modifier.height(8.dp))

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    TextButton(
                        text = denyText,
                        onClick = { viewModel.denyPressed() },
                        modifier = Modifier.weight(1f),
                        cornerRadius = 12.dp,
                        minHeight = 40.dp,
                        insideMargin = PaddingValues(horizontal = 16.dp, vertical = 8.dp),
                    )
                    TextButton(
                        text = stringResource(CoreR.string.grant),
                        enabled = grantEnabled,
                        colors = ButtonDefaults.textButtonColorsPrimary(),
                        onClick = { viewModel.grantPressed() },
                        cornerRadius = 12.dp,
                        minHeight = 40.dp,
                        insideMargin = PaddingValues(horizontal = 16.dp, vertical = 8.dp),
                        modifier = Modifier
                            .weight(1f)
                            .then(
                                if (viewModel.useTapjackProtection) {
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
                    )
                }
            }
        }
    }
}
