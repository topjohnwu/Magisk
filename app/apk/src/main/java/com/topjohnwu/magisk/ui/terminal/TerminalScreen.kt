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
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
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
    modifier: Modifier = Modifier,
    onEmulatorCreated: (TerminalEmulator) -> Unit = {},
) {
    val density = LocalDensity.current
    val renderer = remember {
        val textSizePx = with(density) { 12.sp.toPx().toInt() }
        TerminalRenderer(textSizePx, Typeface.MONOSPACE)
    }

    var emulator by remember { mutableStateOf<TerminalEmulator?>(null) }
    var updateTick by remember { mutableIntStateOf(0) }
    var topRow by remember { mutableIntStateOf(0) }
    var scrollX by remember { mutableFloatStateOf(0f) }
    var scrolledToBottom by remember { mutableStateOf(true) }

    BoxWithConstraints(modifier = modifier) {
        val widthPx = constraints.maxWidth.toFloat()
        val heightPx = constraints.maxHeight.toFloat()
        val visibleCols = max(4, (widthPx / renderer.fontWidth).toInt())
        val cols = max(MIN_TERMINAL_COLUMNS, visibleCols)
        val rows = max(4, ((heightPx - renderer.fontLineSpacingAndAscent) / renderer.fontLineSpacing).toInt())
        val lineHeight = renderer.fontLineSpacing.toFloat()

        @Suppress("UNUSED_EXPRESSION")
        updateTick
        val maxUsedCol = emulator?.maxUsedColumn ?: 0
        val contentWidthPx = maxOf(widthPx, (maxUsedCol + 2) * renderer.fontWidth)
        val maxScrollX = maxOf(0f, contentWidthPx - widthPx)

        LaunchedEffect(cols, rows) {
            val emu = emulator
            if (emu == null) {
                val newEmu = TerminalEmulator(cols, rows, renderer.fontWidth.toInt(), renderer.fontLineSpacing, null)
                newEmu.onScreenUpdate = {
                    if (scrolledToBottom) topRow = 0
                    updateTick++
                }
                emulator = newEmu
                onEmulatorCreated(newEmu)
            } else {
                emu.resize(cols, rows, renderer.fontWidth.toInt(), renderer.fontLineSpacing)
            }
        }

        LaunchedEffect(maxScrollX) {
            scrollX = scrollX.coerceIn(0f, maxScrollX)
        }

        val activeTranscriptRows = emulator?.screen?.activeTranscriptRows ?: 0
        val verticalScrollableState = rememberScrollableState { delta ->
            val emu = emulator ?: return@rememberScrollableState 0f
            val minTop = -emu.screen.activeTranscriptRows
            val rowDelta = -(delta / lineHeight).toInt()
            if (rowDelta != 0) {
                val newTopRow = (topRow + rowDelta).coerceIn(minTop, 0)
                topRow = newTopRow
                scrolledToBottom = newTopRow >= 0
            }
            delta
        }

        val horizontalScrollableState = rememberScrollableState { delta ->
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
                    activeTranscriptRows = activeTranscriptRows,
                    topRow = topRow,
                    visibleRows = rows,
                    onScrollToRow = { newRow ->
                        topRow = newRow
                        scrolledToBottom = newRow >= 0
                    },
                    isScrollInProgress = verticalScrollableState.isScrollInProgress,
                )
                .terminalHorizontalScrollbar(
                    scrollX = scrollX,
                    maxScrollX = maxScrollX,
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
                    @Suppress("UNUSED_EXPRESSION")
                    updateTick
                    val emu = emulator ?: return@drawBehind
                    drawIntoCanvas { canvas ->
                        canvas.nativeCanvas.withTranslation(-scrollX, 0f) {
                            renderer.render(emu, this, topRow, -1, -1, -1, -1)
                        }
                    }
                }
        )
    }
}
