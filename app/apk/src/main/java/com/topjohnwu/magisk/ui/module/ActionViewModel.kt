package com.topjohnwu.magisk.ui.module

import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.terminal.TerminalEmulator
import com.topjohnwu.magisk.terminal.runSuCommand
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

    var actionId: String = ""
    var actionName: String = ""

    val emulator = TerminalEmulator(
        columns = 256,
        rows = 40,
        cellWidthPixels = 0,
        cellHeightPixels = 0,
        transcriptRows = null,
    )

    private val terminalReady = CompletableDeferred<Unit>()

    /** Called by the UI once the terminal has been sized to its viewport. */
    fun onTerminalResized() {
        terminalReady.complete(Unit)
    }

    fun startRunAction() {
        viewModelScope.launch {
            // Start the command with the real terminal size
            terminalReady.await()
            val columns = emulator.mColumns
            val rows = emulator.mRows
            val success = withContext(Dispatchers.IO) {
                runSuCommand(
                    emulator, columns, rows,
                    "cd /data/adb/modules/$actionId && sh ./action.sh"
                )
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
                val transcript = emulator.screen.transcriptText
                writer.write(transcript)
            }
            showToast(file.toString())
        }
    }
}
