package com.topjohnwu.magisk

import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils

object Info {

    @JvmStatic
    var magiskVersionCode = -1

    @JvmStatic
    var magiskVersionString = ""

    var remote = UpdateInfo()

    var keepVerity = false
    var keepEnc = false
    var recovery = false

    fun loadMagiskInfo() {
        runCatching {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":".toRegex())[0]
            magiskVersionCode = ShellUtils.fastCmd("magisk -V").toInt()
            Config.magiskHide = Shell.su("magiskhide --status").exec().isSuccess
        }
    }
}
