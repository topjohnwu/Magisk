package com.topjohnwu.magisk.ui.module

import androidx.lifecycle.viewModelScope
import com.termux.terminal.TerminalSession
import com.termux.view.TerminalView
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.ktx.synchronized
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ui.terminal.TerminalSessionCallback
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class ActionViewModel : BaseViewModel() {

    enum class State {
        RUNNING, SUCCESS, FAILED
    }

    private val _actionState = MutableStateFlow(State.RUNNING)
    val actionState: StateFlow<State> = _actionState.asStateFlow()

    private val _termSession = MutableStateFlow<TerminalSession?>(null)
    val termSession: StateFlow<TerminalSession?> = _termSession.asStateFlow()

    var actionId: String = ""
    var actionName: String = ""

    private val logItems = mutableListOf<String>().synchronized()
    private val emulatorReady = CompletableDeferred<Unit>()

    val sessionCallback = TerminalSessionCallback()

    fun setTerminalView(view: TerminalView) {
        sessionCallback.terminalView = view
    }

    fun onEmulatorReady() {
        if (!emulatorReady.isCompleted) {
            emulatorReady.complete(Unit)
        }
    }

    fun startRunAction() {
        viewModelScope.launch {
            val session = TerminalSession(
                "/system/bin/sh", "/",
                arrayOf("sh", "-c", "exec sleep 2147483647"),
                arrayOf("TERM=xterm-256color"),
                5000, sessionCallback
            )
            _termSession.value = session
            emulatorReady.await()

            val ptyPath = withContext(Dispatchers.IO) {
                Shell.cmd("readlink /proc/${session.pid}/fd/0").exec().out.firstOrNull()
            }
            if (ptyPath == null) {
                _actionState.value = State.FAILED
                return@launch
            }

            val success = withContext(Dispatchers.IO) {
                Shell.cmd(
                    "(export TERM=xterm-256color; run_action '$actionId') >$ptyPath 2>&1"
                ).exec().isSuccess
            }

            _actionState.value = if (success) State.SUCCESS else State.FAILED
        }
    }

    fun saveLog() {
        viewModelScope.launch(Dispatchers.IO) {
            val name = "%s_action_log_%s.log".format(
                actionName,
                System.currentTimeMillis().toTime(timeFormatStandard)
            )
            val file = MediaStoreUtils.getFile(name)
            file.uri.outputStream().bufferedWriter().use { writer ->
                val transcript = _termSession.value?.emulator?.screen?.transcriptText
                if (transcript != null) {
                    writer.write(transcript)
                } else {
                    synchronized(logItems) {
                        logItems.forEach {
                            writer.write(it)
                            writer.newLine()
                        }
                    }
                }
            }
            showSnackbar(file.toString())
        }
    }

    override fun onCleared() {
        super.onCleared()
        _termSession.value?.finishIfRunning()
    }
}
