package com.topjohnwu.magisk

import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.magisk.utils.CachedValue
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils

object Info {

    val envRef = CachedValue { loadState() }

    val env by envRef          // Local
    var remote = UpdateInfo()  // Remote

    var keepVerity = false
    var keepEnc = false
    var recovery = false

    private fun loadState() = runCatching {
        val str = ShellUtils.fastCmd("magisk -v").split(":".toRegex())[0]
        val code = ShellUtils.fastCmd("magisk -V").toInt()
        val hide = Shell.su("magiskhide --status").exec().isSuccess
        var mode = Int.MAX_VALUE
        if (code >= Const.Version.CONNECT_MODE)
            mode = Shell.su("magisk --connect-mode").exec().code
        Env(code, str, hide, mode)
    }.getOrElse { Env() }

    class Env(
        val magiskVersionCode: Int = -1,
        val magiskVersionString: String = "",
        hide: Boolean = false,
        var connectionMode: Int = Int.MAX_VALUE
    ) {
        val magiskHide get() = Config.magiskHide

        init {
            Config.magiskHide = hide
        }
    }
}
