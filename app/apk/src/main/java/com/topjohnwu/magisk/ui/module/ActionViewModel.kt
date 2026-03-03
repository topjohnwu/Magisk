package com.topjohnwu.magisk.ui.module

import androidx.compose.runtime.mutableStateListOf
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.ktx.synchronized
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.IOException

class ActionViewModel : BaseViewModel() {

    enum class State {
        RUNNING, SUCCESS, FAILED
    }

    private val _state = MutableLiveData(State.RUNNING)
    val state: LiveData<State> get() = _state

    private val _actionState = MutableStateFlow(State.RUNNING)
    val actionState: StateFlow<State> = _actionState.asStateFlow()

    val consoleItems = mutableStateListOf<String>()
    lateinit var args: ActionFragmentArgs

    private val logItems = mutableListOf<String>().synchronized()
    private val outItems = object : CallbackList<String>() {
        override fun onAddElement(e: String?) {
            e ?: return
            consoleItems.add(e)
            logItems.add(e)
        }
    }

    fun startRunAction() = viewModelScope.launch {
        onResult(withContext(Dispatchers.IO) {
            try {
                Shell.cmd("run_action '${args.id}'")
                    .to(outItems, logItems)
                    .exec().isSuccess
            } catch (e: IOException) {
                Timber.e(e)
                false
            }
        })
    }

    private fun onResult(success: Boolean) {
        val newState = if (success) State.SUCCESS else State.FAILED
        _state.value = newState
        _actionState.value = newState
    }

    fun saveLog() = withExternalRW {
        viewModelScope.launch(Dispatchers.IO) {
            val name = "%s_action_log_%s.log".format(
                args.name,
                System.currentTimeMillis().toTime(timeFormatStandard)
            )
            val file = MediaStoreUtils.getFile(name)
            file.uri.outputStream().bufferedWriter().use { writer ->
                synchronized(logItems) {
                    logItems.forEach {
                        writer.write(it)
                        writer.newLine()
                    }
                }
            }
            SnackbarEvent(file.toString()).publish()
        }
    }
}
