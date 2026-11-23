package com.topjohnwu.magisk.wear.module

import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshotFlow
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.rememberTextMeasurer
import androidx.wear.compose.foundation.rotary.RotaryScrollableDefaults
import androidx.wear.compose.foundation.rotary.rotaryScrollable
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.terminal.TerminalEmulator
import kotlinx.coroutines.launch
import kotlin.math.max

// The emulator needs at least 2 of each; fewer than this is unreadable anyway
private const val MIN_COLUMNS = 16
private const val MIN_ROWS = 4

/**
 * The output of [emu] as plain monospace text, scrolled by touch and the rotary input, and
 * following new output while it is scrolled to the end.
 *
 * Unlike the phone terminal, there is no horizontal scrolling: a horizontal swipe dismisses the
 * screen. So the emulator is sized to the width that fits, and long lines wrap in the emulator.
 * [onResized] is called once the emulator has that size, so the command can be started with it.
 */
@Composable
fun TerminalText(
    emu: TerminalEmulator,
    modifier: Modifier = Modifier,
    onResized: () -> Unit = {},
) {
    val currentOnResized by rememberUpdatedState(onResized)
    val style = MaterialTheme.typography.bodySmall.copy(fontFamily = FontFamily.Monospace)
    val measurer = rememberTextMeasurer()
    val cell = remember(style) { measurer.measure("M", style).size }

    var text by remember { mutableStateOf("") }

    // Re-read the buffer after new output or a resize. Only called on the main thread, where
    // the emulator is written.
    fun refresh() {
        val screen = emu.screen
        // Rows wrapped by the emulator stay separate lines: they already fit the width
        text = screen.getSelectedText(
            0, -screen.activeTranscriptRows, screen.columns, screen.screenRows, joinBackLines = false
        ).trimEnd()
    }

    DisposableEffect(emu) {
        refresh()
        emu.onScreenUpdate = { refresh() }
        onDispose {
            emu.onScreenUpdate = null
        }
    }

    BoxWithConstraints(modifier = modifier) {
        // Inside an unbounded parent (e.g. a scrolling list), fall back to the minimum
        val columns = if (constraints.hasBoundedWidth) {
            max(MIN_COLUMNS, constraints.maxWidth / max(1, cell.width))
        } else MIN_COLUMNS
        val rows = if (constraints.hasBoundedHeight) {
            max(MIN_ROWS, constraints.maxHeight / max(1, cell.height))
        } else MIN_ROWS

        LaunchedEffect(columns, rows) {
            emu.resize(columns, rows, cell.width, cell.height)
            // The emulator outlives this screen (e.g. across rotation), and the text was read
            // before this resize, so read it again with the new geometry
            refresh()
            currentOnResized()
        }

        val scrollState = rememberScrollState()
        // Follow new output, unless the user has scrolled away from the end
        LaunchedEffect(scrollState) {
            var following = true
            var lastMax = scrollState.maxValue
            snapshotFlow { scrollState.value to scrollState.maxValue }.collect { (value, maxValue) ->
                if (maxValue != lastMax) {
                    lastMax = maxValue
                    // A child job: when a user scroll interrupts it, only it is cancelled
                    if (following) launch { scrollState.scrollTo(maxValue) }
                } else if (maxValue != Int.MAX_VALUE) {
                    // Int.MAX_VALUE until the first layout, which says nothing about the user
                    following = value >= maxValue
                }
            }
        }

        val focusRequester = remember { FocusRequester() }
        LaunchedEffect(Unit) {
            focusRequester.requestFocus()
        }

        Column(
            modifier = Modifier
                .fillMaxWidth()
                .rotaryScrollable(
                    behavior = RotaryScrollableDefaults.behavior(scrollState),
                    focusRequester = focusRequester,
                )
                .verticalScroll(scrollState)
        ) {
            Text(
                text = text,
                style = style,
                color = MaterialTheme.colorScheme.onSurface,
            )
        }
    }
}
