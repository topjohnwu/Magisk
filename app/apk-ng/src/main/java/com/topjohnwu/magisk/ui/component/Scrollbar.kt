package com.topjohnwu.magisk.ui.component

import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.animateDpAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.ScrollState
import androidx.compose.foundation.gestures.awaitEachGesture
import androidx.compose.foundation.gestures.awaitFirstDown
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.calculateEndPadding
import androidx.compose.foundation.layout.calculateStartPadding
import androidx.compose.foundation.lazy.LazyListState
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.composed
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalLayoutDirection
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.LayoutDirection
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlin.math.roundToInt

interface ScrollbarStateAdapter {
    val isScrollable: Boolean
    val thumbRatio: Float
    val offsetRatio: Float
    fun scrollTo(fraction: Float)
}

@Composable
fun rememberScrollbarAdapter(scrollState: ScrollState): ScrollbarStateAdapter {
    val coroutineScope = rememberCoroutineScope()
    return remember(scrollState) {
        object : ScrollbarStateAdapter {
            override val isScrollable: Boolean
                get() = scrollState.maxValue > 0 && scrollState.maxValue != Int.MAX_VALUE

            override val thumbRatio: Float
                get() {
                    val total = scrollState.maxValue + scrollState.viewportSize
                    return if (total > 0) {
                        (scrollState.viewportSize.toFloat() / total.toFloat()).coerceIn(0.08f, 1f)
                    } else 1f
                }

            override val offsetRatio: Float
                get() {
                    return if (scrollState.maxValue > 0) {
                        (scrollState.value.toFloat() / scrollState.maxValue.toFloat()).coerceIn(0f, 1f)
                    } else 0f
                }

            override fun scrollTo(fraction: Float) {
                coroutineScope.launch {
                    val target = (fraction.coerceIn(0f, 1f) * scrollState.maxValue).roundToInt()
                    scrollState.scrollTo(target)
                }
            }
        }
    }
}

@Composable
fun rememberScrollbarAdapter(lazyListState: LazyListState): ScrollbarStateAdapter {
    val coroutineScope = rememberCoroutineScope()
    return remember(lazyListState) {
        object : ScrollbarStateAdapter {
            override val isScrollable: Boolean
                get() {
                    val layoutInfo = lazyListState.layoutInfo
                    val totalItems = layoutInfo.totalItemsCount
                    if (totalItems == 0) return false
                    val visibleItems = layoutInfo.visibleItemsInfo
                    if (visibleItems.isEmpty()) return false
                    if (visibleItems.size < totalItems) return true
                    val viewportHeight = layoutInfo.viewportEndOffset - layoutInfo.viewportStartOffset
                    val totalVisibleHeight = visibleItems.sumOf { it.size }
                    return totalVisibleHeight > viewportHeight
                }

            override val thumbRatio: Float
                get() {
                    val layoutInfo = lazyListState.layoutInfo
                    val totalItems = layoutInfo.totalItemsCount
                    val visibleItems = layoutInfo.visibleItemsInfo
                    if (totalItems == 0 || visibleItems.isEmpty()) return 1f
                    val viewportHeight = (layoutInfo.viewportEndOffset - layoutInfo.viewportStartOffset).toFloat()
                    val avgItemSize = visibleItems.sumOf { it.size }.toFloat() / visibleItems.size
                    val totalEstimatedHeight = avgItemSize * totalItems
                    return if (totalEstimatedHeight > 0f) {
                        (viewportHeight / totalEstimatedHeight).coerceIn(0.08f, 1f)
                    } else 1f
                }

            override val offsetRatio: Float
                get() {
                    val layoutInfo = lazyListState.layoutInfo
                    val totalItems = layoutInfo.totalItemsCount
                    val visibleItems = layoutInfo.visibleItemsInfo
                    if (totalItems == 0 || visibleItems.isEmpty()) return 0f
                    val viewportHeight = (layoutInfo.viewportEndOffset - layoutInfo.viewportStartOffset).toFloat()
                    val avgItemSize = visibleItems.sumOf { it.size }.toFloat() / visibleItems.size
                    val totalEstimatedHeight = avgItemSize * totalItems
                    val maxOffset = (totalEstimatedHeight - viewportHeight).coerceAtLeast(1f)
                    val currentOffset = lazyListState.firstVisibleItemIndex * avgItemSize + lazyListState.firstVisibleItemScrollOffset
                    return (currentOffset / maxOffset).coerceIn(0f, 1f)
                }

            override fun scrollTo(fraction: Float) {
                coroutineScope.launch {
                    val totalItems = lazyListState.layoutInfo.totalItemsCount
                    if (totalItems > 0) {
                        val targetIndex = (fraction.coerceIn(0f, 1f) * (totalItems - 1)).roundToInt().coerceIn(0, totalItems - 1)
                        lazyListState.scrollToItem(targetIndex)
                    }
                }
            }
        }
    }
}

@Composable
fun rememberTerminalScrollbarAdapter(
    activeTranscriptRows: Int,
    topRow: Int,
    visibleRows: Int,
    onScrollToRow: (Int) -> Unit
): ScrollbarStateAdapter {
    val currentOnScroll = rememberUpdatedState(onScrollToRow)
    return remember(activeTranscriptRows, topRow, visibleRows) {
        object : ScrollbarStateAdapter {
            override val isScrollable: Boolean
                get() = activeTranscriptRows > 0

            override val thumbRatio: Float
                get() {
                    val totalRows = activeTranscriptRows + visibleRows
                    return if (totalRows > 0) {
                        (visibleRows.toFloat() / totalRows.toFloat()).coerceIn(0.08f, 1f)
                    } else 1f
                }

            override val offsetRatio: Float
                get() {
                    return if (activeTranscriptRows > 0) {
                        ((activeTranscriptRows + topRow).toFloat() / activeTranscriptRows.toFloat()).coerceIn(0f, 1f)
                    } else 0f
                }

            override fun scrollTo(fraction: Float) {
                if (activeTranscriptRows > 0) {
                    val target = (-activeTranscriptRows * (1f - fraction.coerceIn(0f, 1f))).roundToInt().coerceIn(-activeTranscriptRows, 0)
                    currentOnScroll.value(target)
                }
            }
        }
    }
}

fun Modifier.verticalScrollbar(
    state: ScrollState,
    contentPadding: PaddingValues = PaddingValues(0.dp),
    thumbColor: Color? = null,
): Modifier = composed {
    val adapter = rememberScrollbarAdapter(state)
    verticalScrollbar(
        adapter = adapter,
        isScrollInProgress = state.isScrollInProgress,
        contentPadding = contentPadding,
        thumbColor = thumbColor,
    )
}

fun Modifier.verticalScrollbar(
    state: LazyListState,
    contentPadding: PaddingValues = PaddingValues(0.dp),
    thumbColor: Color? = null,
): Modifier = composed {
    val adapter = rememberScrollbarAdapter(state)
    verticalScrollbar(
        adapter = adapter,
        isScrollInProgress = state.isScrollInProgress,
        contentPadding = contentPadding,
        thumbColor = thumbColor,
    )
}

fun Modifier.terminalScrollbar(
    activeTranscriptRows: Int,
    topRow: Int,
    visibleRows: Int,
    onScrollToRow: (Int) -> Unit,
    isScrollInProgress: Boolean = false,
    contentPadding: PaddingValues = PaddingValues(0.dp),
    thumbColor: Color? = null,
): Modifier = composed {
    val adapter = rememberTerminalScrollbarAdapter(
        activeTranscriptRows = activeTranscriptRows,
        topRow = topRow,
        visibleRows = visibleRows,
        onScrollToRow = onScrollToRow
    )
    verticalScrollbar(
        adapter = adapter,
        isScrollInProgress = isScrollInProgress,
        contentPadding = contentPadding,
        thumbColor = thumbColor,
    )
}

fun Modifier.verticalScrollbar(
    adapter: ScrollbarStateAdapter,
    isScrollInProgress: Boolean = false,
    contentPadding: PaddingValues = PaddingValues(0.dp),
    thumbColor: Color? = null,
    minThumbHeight: Dp = 36.dp,
    hitTargetWidth: Dp = 32.dp,
): Modifier = composed {
    val layoutDirection = LocalLayoutDirection.current
    val resolvedThumbColor = thumbColor ?: MaterialTheme.colorScheme.onSurfaceVariant

    var isDragging by remember { mutableStateOf(false) }
    val alphaAnim = remember { Animatable(0f) }

    val isScrollable by remember(adapter) { derivedStateOf { adapter.isScrollable } }

    LaunchedEffect(isScrollInProgress, isDragging, isScrollable) {
        if (!isScrollable) {
            alphaAnim.snapTo(0f)
            return@LaunchedEffect
        }
        if (isScrollInProgress || isDragging) {
            alphaAnim.animateTo(1f, animationSpec = tween(150))
        } else {
            delay(1500)
            alphaAnim.animateTo(0f, animationSpec = tween(500))
        }
    }

    val animatedThickness by animateDpAsState(
        targetValue = if (isDragging) 6.dp else 4.dp,
        label = "scrollbar_thickness"
    )

    this
        .pointerInput(adapter, contentPadding, layoutDirection, isScrollable) {
            if (!isScrollable) return@pointerInput

            val paddingTop = contentPadding.calculateTopPadding().toPx()
            val paddingBottom = contentPadding.calculateBottomPadding().toPx()
            val paddingStart = contentPadding.calculateStartPadding(layoutDirection).toPx()
            val paddingEnd = contentPadding.calculateEndPadding(layoutDirection).toPx()
            val hitWidthPx = hitTargetWidth.toPx()
            val minThumbHeightPx = minThumbHeight.toPx()

            fun calculateFraction(y: Float, height: Float, thumbH: Float): Float {
                val trackHeight = height - paddingTop - paddingBottom
                val availableHeight = trackHeight - thumbH
                return if (availableHeight > 0f) {
                    ((y - paddingTop - thumbH / 2f) / availableHeight).coerceIn(0f, 1f)
                } else 0f
            }

            awaitEachGesture {
                val down = awaitFirstDown(requireUnconsumed = false)
                val width = size.width.toFloat()
                val height = size.height.toFloat()

                val isHit = when (layoutDirection) {
                    LayoutDirection.Rtl -> down.position.x <= (paddingStart + hitWidthPx)
                    LayoutDirection.Ltr -> down.position.x >= (width - paddingEnd - hitWidthPx)
                }

                if (isHit && down.position.y in paddingTop..(height - paddingBottom)) {
                    down.consume()
                    isDragging = true
                    val trackHeight = height - paddingTop - paddingBottom
                    val thumbH = (trackHeight * adapter.thumbRatio).coerceAtLeast(minThumbHeightPx)
                    adapter.scrollTo(calculateFraction(down.position.y, height, thumbH))

                    val pointerId = down.id
                    while (true) {
                        val event = awaitPointerEvent()
                        val change = event.changes.firstOrNull { it.id == pointerId } ?: break
                        if (!change.pressed) break
                        change.consume()
                        adapter.scrollTo(calculateFraction(change.position.y, height, thumbH))
                    }
                    isDragging = false
                }
            }
        }
        .drawWithContent {
            drawContent()

            val alpha = alphaAnim.value
            if (alpha > 0f && isScrollable) {
                val paddingTop = contentPadding.calculateTopPadding().toPx()
                val paddingBottom = contentPadding.calculateBottomPadding().toPx()
                val paddingStart = contentPadding.calculateStartPadding(layoutDirection).toPx()
                val paddingEnd = contentPadding.calculateEndPadding(layoutDirection).toPx()
                val minThumbHeightPx = minThumbHeight.toPx()
                val thicknessPx = animatedThickness.toPx()

                val trackHeight = size.height - paddingTop - paddingBottom
                if (trackHeight > 0f) {
                    val thumbHeight = (trackHeight * adapter.thumbRatio).coerceAtLeast(minThumbHeightPx)
                    val thumbOffset = paddingTop + (trackHeight - thumbHeight) * adapter.offsetRatio

                    val left = when (layoutDirection) {
                        LayoutDirection.Rtl -> paddingStart + 2.dp.toPx()
                        LayoutDirection.Ltr -> size.width - paddingEnd - 2.dp.toPx() - thicknessPx
                    }

                    drawRoundRect(
                        color = (if (isDragging) resolvedThumbColor else resolvedThumbColor.copy(alpha = 0.5f)),
                        topLeft = Offset(left, thumbOffset),
                        size = Size(thicknessPx, thumbHeight),
                        cornerRadius = CornerRadius(thicknessPx / 2f, thicknessPx / 2f),
                        alpha = alpha
                    )
                }
            }
        }
}
