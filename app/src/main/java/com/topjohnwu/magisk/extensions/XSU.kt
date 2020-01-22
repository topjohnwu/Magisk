package com.topjohnwu.magisk.extensions

import com.topjohnwu.magisk.core.Info
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFileInputStream
import com.topjohnwu.superuser.io.SuFileOutputStream
import java.io.File

fun reboot(reason: String = if (Info.recovery) "recovery" else "") {
    Shell.su("/system/bin/svc power reboot $reason || /system/bin/reboot $reason").submit()
}

fun File.suOutputStream() = SuFileOutputStream(this)
fun File.suInputStream() = SuFileInputStream(this)

val hasRoot get() = Shell.rootAccess()
