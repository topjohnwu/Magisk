package com.topjohnwu.magisk.core.utils

import android.content.Context
import android.os.Build
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ktx.rawResource
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.io.SuFile

class RootInit : Shell.Initializer() {

    override fun onInit(context: Context, shell: Shell): Boolean {
        return init(context.wrap(), shell)
    }

    fun init(context: Context, shell: Shell): Boolean {
        shell.newJob().apply {
            if (Const.Version.atLeast_20_4()) {
                add("export MAGISKTMP=$(magisk --path)/.magisk")
            } else {
                add("export MAGISKTMP=/sbin/.magisk")
            }
            if (Const.Version.atLeastCanary()) {
                add("export ASH_STANDALONE=1")
                add("[ -x /data/adb/magisk/busybox ] && exec /data/adb/magisk/busybox sh")
            } else {
                add("export PATH=\"\$MAGISKTMP/busybox:\$PATH\"")
            }
            add(context.rawResource(R.raw.manager))
            if (shell.isRoot) {
                add(context.rawResource(R.raw.util_functions))
            }
            add("mm_init")
        }.exec()

        fun getvar(name: String) = ShellUtils.fastCmd(shell, "echo \$$name")
        fun getBool(name: String) = getvar(name).toBoolean()

        Const.MAGISKTMP = getvar("MAGISKTMP")
        Info.isSAR = getBool("SYSTEM_ROOT")
        Info.ramdisk = getBool("RAMDISKEXIST")
        Info.isAB = getBool("ISAB")

        // FBE does not exist pre 7.0
        if (Build.VERSION.SDK_INT >= 24)
            Info.isFBE = SuFile("/data/unencrypted").exists()

        // Default presets
        Config.recovery = getBool("RECOVERYMODE")
        Config.keepVerity = getBool("KEEPVERITY")
        Config.keepEnc = getBool("KEEPFORCEENCRYPT")

        return true
    }
}
