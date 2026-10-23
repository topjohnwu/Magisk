package com.topjohnwu.magisk.ui.terminal

import android.graphics.Typeface
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.Orientation
import androidx.compose.foundation.gestures.rememberScrollableState
import androidx.compose.foundation.gestures.scrollable
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableLongStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshotFlow
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clipToBounds
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.drawIntoCanvas
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.sp
import androidx.core.graphics.withTranslation
import com.topjohnwu.magisk.terminal.TerminalEmulator
import com.topjohnwu.magisk.ui.component.terminalHorizontalScrollbar
import com.topjohnwu.magisk.ui.component.terminalScrollbar
import kotlin.math.max

private const val MIN_TERMINAL_COLUMNS = 256

@Composable
fun TerminalScreen(
    emulator: TerminalEmulator,
    modifier: Modifier = Modifier,
    onResized: () -> Unit = {},
) {
    val currentOnResized by rememberUpdatedState(onResized)
    val density = LocalDensity.current
    val renderer = remember {
        val textSizePx = with(density) { 12.sp.toPx().toInt() }
        TerminalRenderer(textSizePx, Typeface.MONOSPACE)
    }

    var drawTrigger by remember { mutableLongStateOf(0L) }
    var topRow by remember { mutableIntStateOf(0) }
    var scrollX by remember { mutableFloatStateOf(0f) }
    var scrolledToBottom by remember { mutableStateOf(true) }
    // Snapshot copies of emulator fields so scrollbars observe output changes
    var transcriptRows by remember { mutableIntStateOf(0) }
    var maxUsedColumn by remember { mutableIntStateOf(0) }

    // Re-read emulator state after new output or a resize, and redraw
    fun refresh() {
        transcriptRows = emulator.screen.activeTranscriptRows
        maxUsedColumn = emulator.maxUsedColumn
        topRow = if (scrolledToBottom) 0 else topRow.coerceAtLeast(-transcriptRows)
        drawTrigger++
    }

    DisposableEffect(emulator) {
        refresh()
        emulator.onScreenUpdate = { refresh() }
        onDispose {
            emulator.onScreenUpdate = null
        }
    }

    BoxWithConstraints(modifier = modifier) {
        val widthPx = constraints.maxWidth.toFloat()
        val heightPx = constraints.maxHeight.toFloat()
        val visibleCols = max(4, (widthPx / renderer.fontWidth).toInt())
        val cols = max(MIN_TERMINAL_COLUMNS, visibleCols)
        val rows = max(4, ((heightPx - renderer.fontLineSpacingAndAscent) / renderer.fontLineSpacing).toInt())
        val lineHeight = renderer.fontLineSpacing.toFloat()

        fun getMaxScrollX(): Float {
            val contentWidthPx = maxOf(widthPx, (maxUsedColumn + 2) * renderer.fontWidth)
            return maxOf(0f, contentWidthPx - widthPx)
        }

        LaunchedEffect(cols, rows) {
            emulator.resize(cols, rows, renderer.fontWidth.toInt(), renderer.fontLineSpacing)
            // The emulator outlives this screen (e.g. across rotation), and the first frame was drawn
            // before this resize, so redraw with the new geometry.
            refresh()
            currentOnResized()
        }

        // Keep scrollX in range when the viewport or content width changes
        LaunchedEffect(widthPx) {
            snapshotFlow { getMaxScrollX() }.collect { maxScrollX ->
                scrollX = scrollX.coerceIn(0f, maxScrollX)
            }
        }

        val verticalScrollableState = rememberScrollableState { delta ->
            val minTop = -emulator.screen.activeTranscriptRows
            val rowDelta = -(delta / lineHeight).toInt()
            if (rowDelta != 0) {
                val newTopRow = (topRow + rowDelta).coerceIn(minTop, 0)
                topRow = newTopRow
                scrolledToBottom = newTopRow >= 0
            }
            delta
        }

        val horizontalScrollableState = rememberScrollableState { delta ->
            val maxScrollX = getMaxScrollX()
            val oldScrollX = scrollX
            val newScrollX = (scrollX - delta).coerceIn(0f, maxScrollX)
            scrollX = newScrollX
            -(newScrollX - oldScrollX)
        }

        Spacer(
            modifier = Modifier
                .fillMaxSize()
                .clipToBounds()
                .background(Color.Black)
                .terminalScrollbar(
                    activeTranscriptRowsProvider = { transcriptRows },
                    topRowProvider = { topRow },
                    visibleRows = rows,
                    onScrollToRow = { newRow ->
                        topRow = newRow
                        scrolledToBottom = newRow >= 0
                    },
                    isScrollInProgress = verticalScrollableState.isScrollInProgress,
                )
                .terminalHorizontalScrollbar(
                    scrollXProvider = { scrollX },
                    maxScrollXProvider = { getMaxScrollX() },
                    viewportWidth = widthPx,
                    onScrollToX = { newScrollX -> scrollX = newScrollX },
                    isScrollInProgress = horizontalScrollableState.isScrollInProgress,
                )
                .scrollable(
                    orientation = Orientation.Vertical,
                    state = verticalScrollableState
                )
                .scrollable(
                    orientation = Orientation.Horizontal,
                    state = horizontalScrollableState
                )
                .drawBehind {
                    if (drawTrigger >= 0L) {
                        drawIntoCanvas { canvas ->
                            canvas.nativeCanvas.withTranslation(-scrollX, 0f) {
                                renderer.render(emulator, this, topRow, -1, -1, -1, -1)
                            }
                        }
                    }
                }
        )
    }
}
