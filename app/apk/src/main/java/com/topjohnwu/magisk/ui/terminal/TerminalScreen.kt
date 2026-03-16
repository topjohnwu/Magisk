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
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.drawIntoCanvas
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.sp
import com.topjohnwu.magisk.terminal.TerminalEmulator
import kotlin.math.max

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
    var scrolledToBottom by remember { mutableStateOf(true) }

    BoxWithConstraints(modifier = modifier) {
        val widthPx = constraints.maxWidth
        val heightPx = constraints.maxHeight
        val cols = max(4, (widthPx / renderer.fontWidth).toInt())
        val rows = max(4, (heightPx - renderer.fontLineSpacingAndAscent) / renderer.fontLineSpacing)
        val lineHeight = renderer.fontLineSpacing.toFloat()

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

        Spacer(
            modifier = Modifier
                .fillMaxSize()
                .background(Color.Black)
                .scrollable(
                    orientation = Orientation.Vertical,
                    state = rememberScrollableState { delta ->
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
                )
                .drawBehind {
                    @Suppress("UNUSED_EXPRESSION")
                    updateTick
                    val emu = emulator ?: return@drawBehind
                    drawIntoCanvas { canvas ->
                        renderer.render(emu, canvas.nativeCanvas, topRow, -1, -1, -1, -1)
                    }
                }
        )
    }
}
