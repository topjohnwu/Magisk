package com.topjohnwu.magisk.ui.terminal

import android.graphics.Color
import android.graphics.Typeface
import android.util.TypedValue
import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView
import com.termux.terminal.TerminalSession
import com.termux.view.TerminalView
import com.termux.view.TerminalViewClient

@Composable
fun TerminalComposeView(
    session: TerminalSession?,
    modifier: Modifier = Modifier,
    onViewCreated: (TerminalView) -> Unit = {},
    onEmulatorReady: () -> Unit = {},
    onViewDisposed: () -> Unit = {},
) {
    DisposableEffect(Unit) {
        onDispose { onViewDisposed() }
    }
    AndroidView(
        factory = { context ->
            val textSizePx = TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_SP, 12f, context.resources.displayMetrics
            ).toInt()
            TerminalView(context, null).apply {
                setBackgroundColor(Color.BLACK)
                setTextSize(textSizePx)
                setTypeface(Typeface.MONOSPACE)
                keepScreenOn = true
                setTerminalViewClient(ReadOnlyTerminalViewClient(onEmulatorReady))
                onViewCreated(this)
            }
        },
        update = { view ->
            if (session != null && view.mTermSession != session) {
                view.attachSession(session)
            }
        },
        modifier = modifier,
    )
}

private class ReadOnlyTerminalViewClient(
    private val onEmulatorReady: () -> Unit,
) : TerminalViewClient {
    override fun onScale(scale: Float) = 1.0f
    override fun onSingleTapUp(e: MotionEvent) {}
    override fun shouldBackButtonBeMappedToEscape() = false
    override fun shouldEnforceCharBasedInput() = false
    override fun shouldUseCtrlSpaceWorkaround() = false
    override fun isTerminalViewSelected() = true
    override fun copyModeChanged(copyMode: Boolean) {}
    override fun onKeyDown(keyCode: Int, e: KeyEvent, session: TerminalSession) = false
    override fun onKeyUp(keyCode: Int, e: KeyEvent) = false
    override fun onLongPress(event: MotionEvent) = false
    override fun readControlKey() = false
    override fun readAltKey() = false
    override fun readShiftKey() = false
    override fun readFnKey() = false
    override fun onCodePoint(codePoint: Int, ctrlDown: Boolean, session: TerminalSession) = false
    override fun onEmulatorSet() {
        onEmulatorReady()
    }

    override fun logError(tag: String, message: String) {}
    override fun logWarn(tag: String, message: String) {}
    override fun logInfo(tag: String, message: String) {}
    override fun logDebug(tag: String, message: String) {}
    override fun logVerbose(tag: String, message: String) {}
    override fun logStackTraceWithMessage(tag: String, message: String, e: Exception) {}
    override fun logStackTrace(tag: String, e: Exception) {}
}
