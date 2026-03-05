package com.topjohnwu.magisk.ui.flash

import android.net.Uri
import androidx.compose.runtime.mutableStateListOf
import androidx.core.net.toFile
import androidx.lifecycle.viewModelScope
import com.termux.terminal.TerminalSession
import com.termux.view.TerminalView
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.synchronized
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ui.terminal.TerminalSessionCallback
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

class FlashViewModel : BaseViewModel() {

    enum class State {
        FLASHING, SUCCESS, FAILED
    }

    private val _flashState = MutableStateFlow(State.FLASHING)
    val flashState: StateFlow<State> = _flashState.asStateFlow()

    private val _showReboot = MutableStateFlow(Info.isRooted)
    val showReboot: StateFlow<Boolean> = _showReboot.asStateFlow()

    var flashAction: String = ""
    var flashUri: Uri? = null

    // --- TerminalView mode (FLASH_ZIP) ---

    private val _termSession = MutableStateFlow<TerminalSession?>(null)
    val termSession: StateFlow<TerminalSession?> = _termSession.asStateFlow()

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

    // --- LazyColumn mode (MagiskInstaller) ---

    val consoleItems = mutableStateListOf<String>()
    private val logItems = mutableListOf<String>().synchronized()
    private val outItems = object : CallbackList<String>() {
        override fun onAddElement(e: String?) {
            e ?: return
            consoleItems.add(e)
            logItems.add(e)
        }
    }

    // --- Shared ---

    fun startFlashing() {
        val action = flashAction
        val uri = flashUri

        viewModelScope.launch {
            when (action) {
                Const.Value.FLASH_ZIP -> {
                    uri ?: return@launch
                    flashZipWithPty(uri)
                }
                Const.Value.UNINSTALL -> {
                    _showReboot.value = false
                    onResult(withContext(Dispatchers.IO) {
                        MagiskInstaller.Uninstall(outItems, logItems).exec()
                    })
                }
                Const.Value.FLASH_MAGISK -> {
                    onResult(withContext(Dispatchers.IO) {
                        if (Info.isEmulator)
                            MagiskInstaller.Emulator(outItems, logItems).exec()
                        else
                            MagiskInstaller.Direct(outItems, logItems).exec()
                    })
                }
                Const.Value.FLASH_INACTIVE_SLOT -> {
                    _showReboot.value = false
                    onResult(withContext(Dispatchers.IO) {
                        MagiskInstaller.SecondSlot(outItems, logItems).exec()
                    })
                }
                Const.Value.PATCH_FILE -> {
                    uri ?: return@launch
                    _showReboot.value = false
                    onResult(withContext(Dispatchers.IO) {
                        MagiskInstaller.Patch(uri, outItems, logItems).exec()
                    })
                }
            }
        }
    }

    private fun onResult(success: Boolean) {
        _flashState.value = if (success) State.SUCCESS else State.FAILED
    }

    private suspend fun flashZipWithPty(uri: Uri) {
        val session = TerminalSession(
            "/system/bin/sh", "/",
            arrayOf("sh", "-c", "exec sleep 2147483647"),
            arrayOf("TERM=xterm-256color"),
            5000, sessionCallback
        )
        _termSession.value = session
        emulatorReady.await()

        val installDir = File(AppContext.cacheDir, "flash")
        val result = withContext(Dispatchers.IO) {
            try {
                installDir.deleteRecursively()
                installDir.mkdirs()

                val zipFile = if (uri.scheme == "file") {
                    uri.toFile()
                } else {
                    File(installDir, "install.zip").also {
                        try {
                            uri.inputStream().writeTo(it)
                        } catch (e: IOException) {
                            val msg = if (e is FileNotFoundException) "Invalid Uri" else "Cannot copy to cache"
                            return@withContext msg to null
                        }
                    }
                }

                val binary = File(installDir, "update-binary")
                AppContext.assets.open("module_installer.sh").use { it.writeTo(binary) }

                val name = uri.displayName
                null to Triple(installDir, zipFile, name)
            } catch (e: IOException) {
                Timber.e(e)
                "Unable to extract files" to null
            }
        }

        val (error, prepResult) = result
        if (prepResult == null) {
            writeToPty(session, "! ${error ?: "Installation failed"}")
            _flashState.value = State.FAILED
            return
        }

        val (dir, zipFile, displayName) = prepResult
        val ptyPath = getPtyPath(session)
        if (ptyPath == null) {
            _flashState.value = State.FAILED
            return
        }

        val success = withContext(Dispatchers.IO) {
            Shell.cmd(
                "(export TERM=xterm-256color; " +
                "echo '- Installing $displayName'; " +
                "sh $dir/update-binary dummy 1 '${zipFile.absolutePath}'; " +
                "EXIT=\$?; " +
                "if [ \$EXIT -ne 0 ]; then echo '! Installation failed'; fi; " +
                "exit \$EXIT) <>$ptyPath >&0 2>&0"
            ).exec().isSuccess
        }

        Shell.cmd("cd /", "rm -rf $dir ${Const.TMPDIR}").submit()
        _flashState.value = if (success) State.SUCCESS else State.FAILED
    }

    private suspend fun getPtyPath(session: TerminalSession): String? {
        return withContext(Dispatchers.IO) {
            Shell.cmd("readlink /proc/${session.pid}/fd/0").exec().out.firstOrNull()
        }
    }

    private suspend fun writeToPty(session: TerminalSession, message: String) {
        val ptyPath = getPtyPath(session) ?: return
        withContext(Dispatchers.IO) {
            Shell.cmd("echo '$message' >$ptyPath").exec()
        }
    }

    fun saveLog() {
        viewModelScope.launch(Dispatchers.IO) {
            val name = "magisk_install_log_%s.log".format(
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

    fun restartPressed() = reboot()

    override fun onCleared() {
        super.onCleared()
        _termSession.value?.finishIfRunning()
    }
}
