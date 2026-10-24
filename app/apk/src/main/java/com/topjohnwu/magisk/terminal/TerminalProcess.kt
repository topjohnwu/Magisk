package com.topjohnwu.magisk.terminal

import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.File

private val busyboxPath: String by lazy {
    Shell.cmd("readlink /proc/self/exe").exec().out.firstOrNull()
        ?: "/data/adb/magisk/busybox"
}

private val terminalScope = CoroutineScope(Dispatchers.Main.immediate)

fun TerminalEmulator.appendOnMain(bytes: ByteArray, len: Int) {
    terminalScope.launch {
        append(bytes, len)
        onScreenUpdate?.invoke()
    }
}

fun TerminalEmulator.appendLineOnMain(line: String) {
    terminalScope.launch {
        val clear = if (inPlaceNewlines > 0) {
            "\r\u001b[${inPlaceNewlines}A\u001b[J"
        } else {
            "\r\u001b[2K"
        }
        val text = if (line.startsWith('\r')) {
            val content = line.substring(1)
            inPlaceNewlines = content.count { it == '\n' }
            "$clear${content.replace("\n", "\r\n")}"
        } else {
            inPlaceNewlines = 0
            "$clear$line\r\n"
        }
        val bytes = text.toByteArray(Charsets.UTF_8)
        append(bytes, bytes.size)
        onScreenUpdate?.invoke()
    }
}

/**
 * Run a command as root inside a PTY (via busybox script).
 * Reads raw bytes from the process and feeds them to the terminal emulator.
 * Must be called from a background thread. [columns] and [rows] should be read from the
 * emulator on the main thread, after the terminal has been sized to its viewport.
 * Returns true if the command exits with code 0.
 */
fun runSuCommand(emulator: TerminalEmulator, columns: Int, rows: Int, command: String): Boolean {
    // busybox script always exits with 0, so have the command write its exit code to a file
    val exitFile = File.createTempFile("exit", null, AppContext.cacheDir)
    return try {
        val wrappedCmd = "export TERM=xterm-256color; stty cols $columns rows $rows 2>/dev/null; " +
            "($command); echo \$? > '${exitFile.path}'"
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

        process.waitFor()
        exitFile.readText().trim() == "0"
    } catch (e: Exception) {
        Timber.e(e, "runSuCommand failed")
        emulator.appendLineOnMain("! Error: ${e.message}")
        false
    } finally {
        exitFile.delete()
    }
}
