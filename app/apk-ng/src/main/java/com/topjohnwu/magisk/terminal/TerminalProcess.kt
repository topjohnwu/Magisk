package com.topjohnwu.magisk.terminal

import android.os.Handler
import android.os.Looper
import com.topjohnwu.superuser.Shell
import timber.log.Timber

private val busyboxPath: String by lazy {
    Shell.cmd("readlink /proc/self/exe").exec().out.firstOrNull()
        ?: "/data/adb/magisk/busybox"
}

private val mainHandler = Handler(Looper.getMainLooper())

fun TerminalEmulator.appendOnMain(bytes: ByteArray, len: Int) {
    mainHandler.post {
        append(bytes, len)
        onScreenUpdate?.invoke()
    }
}

fun TerminalEmulator.appendLineOnMain(line: String) {
    val bytes = "$line\r\n".toByteArray(Charsets.UTF_8)
    appendOnMain(bytes, bytes.size)
}

/**
 * Run a command as root inside a PTY (via busybox script).
 * Reads raw bytes from the process and feeds them to the terminal emulator.
 * Must be called from a background thread.
 * Returns true if the process exits with code 0.
 */
fun runSuCommand(emulator: TerminalEmulator, command: String): Boolean {
    return try {
        val cols = emulator.mColumns
        val rows = emulator.mRows
        val wrappedCmd = "export TERM=xterm-256color; stty cols $cols rows $rows 2>/dev/null; $command"
        val escapedCmd = wrappedCmd.replace("'", "'\\''")

        val process = ProcessBuilder(
            "su", "-c",
            "$busyboxPath script -q -c '$escapedCmd' /dev/null"
        ).redirectErrorStream(true).start()

        process.outputStream.close()

        val buffer = ByteArray(4096)
        process.inputStream.use { input ->
            while (true) {
                val n = input.read(buffer)
                if (n == -1) break
                emulator.appendOnMain(buffer.copyOf(n), n)
            }
        }

        process.waitFor() == 0
    } catch (e: Exception) {
        Timber.e(e, "runSuCommand failed")
        emulator.appendLineOnMain("! Error: ${e.message}")
        false
    }
}
