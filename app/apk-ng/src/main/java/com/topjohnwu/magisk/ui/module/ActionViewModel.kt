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

    private var emulator: TerminalEmulator? = null
    private val emulatorReady = CompletableDeferred<TerminalEmulator>()

    fun onEmulatorCreated(emu: TerminalEmulator) {
        emulator = emu
        emulatorReady.complete(emu)
    }

    fun startRunAction() {
        viewModelScope.launch {
            val emu = emulatorReady.await()

            val success = withContext(Dispatchers.IO) {
                runSuCommand(
                    emu,
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
                val transcript = emulator?.screen?.transcriptText
                if (transcript != null) {
                    writer.write(transcript)
                }
            }
            showSnackbar(file.toString())
        }
    }
}
