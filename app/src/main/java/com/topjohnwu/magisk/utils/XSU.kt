package com.topjohnwu.magisk.utils

import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFileInputStream
import com.topjohnwu.superuser.io.SuFileOutputStream
import java.io.File

fun reboot(recovery: Boolean = false): Shell.Result {
    val command = StringBuilder("/system/bin/reboot")
        .appendIf(recovery) {
            append(" recovery")
        }
        .toString()

    return Shell.su(command).exec()
}

fun File.suOutputStream() = SuFileOutputStream(this)
fun File.suInputStream() = SuFileInputStream(this)