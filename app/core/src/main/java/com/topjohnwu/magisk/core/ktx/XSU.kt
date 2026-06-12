package com.topjohnwu.magisk.core.ktx

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

fun reboot(reason: String = if (Config.recovery) "recovery" else "") {
    if (reason == "recovery") {
        // KEYCODE_POWER = 26, hide incorrect "Factory data reset" message
        Shell.cmd("/system/bin/input keyevent 26").submit()
    }
    Shell.cmd("/system/bin/svc power reboot $reason || /system/bin/reboot $reason").submit()
}

suspend fun Shell.Job.await() = withContext(Dispatchers.IO) { exec() }
