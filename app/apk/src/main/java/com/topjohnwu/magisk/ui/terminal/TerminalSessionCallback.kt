package com.topjohnwu.magisk.ui.terminal

import com.termux.terminal.TerminalSession
import com.termux.terminal.TerminalSessionClient
import com.termux.view.TerminalView
import timber.log.Timber

class TerminalSessionCallback(
    private val onFinished: (TerminalSession) -> Unit = {},
) : TerminalSessionClient {

    var terminalView: TerminalView? = null

    override fun onTextChanged(changedSession: TerminalSession) {
        terminalView?.onScreenUpdated()
    }

    override fun onTitleChanged(changedSession: TerminalSession) {}

    override fun onSessionFinished(finishedSession: TerminalSession) {
        onFinished(finishedSession)
    }

    override fun onCopyTextToClipboard(session: TerminalSession, text: String) {}

    override fun onPasteTextFromClipboard(session: TerminalSession?) {}

    override fun onBell(session: TerminalSession) {}

    override fun onColorsChanged(session: TerminalSession) {}

    override fun onTerminalCursorStateChange(state: Boolean) {}

    override fun getTerminalCursorStyle(): Int = 0

    override fun logError(tag: String, message: String) { Timber.tag(tag).e(message) }
    override fun logWarn(tag: String, message: String) { Timber.tag(tag).w(message) }
    override fun logInfo(tag: String, message: String) { Timber.tag(tag).i(message) }
    override fun logDebug(tag: String, message: String) { Timber.tag(tag).d(message) }
    override fun logVerbose(tag: String, message: String) { Timber.tag(tag).v(message) }

    override fun logStackTraceWithMessage(tag: String, message: String, e: Exception) {
        Timber.tag(tag).e(e, message)
    }

    override fun logStackTrace(tag: String, e: Exception) {
        Timber.tag(tag).e(e)
    }
}
