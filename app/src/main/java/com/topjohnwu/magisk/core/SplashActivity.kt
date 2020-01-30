package com.topjohnwu.magisk.core

import android.app.Activity
import android.content.Context
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.core.view.Shortcuts
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils

open class SplashActivity : Activity() {

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Shell.getShell { initAndStart() }
    }

    private fun initAndStart() {
        val pkg = Config.suManager
        if (Config.suManager.isNotEmpty() && packageName == BuildConfig.APPLICATION_ID) {
            Config.suManager = ""
            Shell.su("pm uninstall $pkg").submit()
        }
        if (pkg == packageName) {
            runCatching {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                packageManager.getApplicationInfo(BuildConfig.APPLICATION_ID, 0)
                Shell.su("pm uninstall " + BuildConfig.APPLICATION_ID).submit()
            }
        }

        Info.keepVerity = ShellUtils.fastCmd("echo \$KEEPVERITY").toBoolean()
        Info.keepEnc = ShellUtils.fastCmd("echo \$KEEPFORCEENCRYPT").toBoolean()
        Info.recovery = ShellUtils.fastCmd("echo \$RECOVERYMODE").toBoolean()

        // Set default configs
        Config.initialize()

        // Create notification channel on Android O
        Notifications.setup(this)

        // Schedule periodic update checks
        Utils.scheduleUpdateCheck(this)

        // Setup shortcuts
        Shortcuts.setup(this)

        if (Info.isNewReboot) {
            val shell = Shell.newInstance()
            shell.newJob().add("mm_patch_dtb").submit {
                if (it.isSuccess)
                    Notifications.dtboPatched(this)
                shell.close()
            }
        }

        DONE = true
        Navigation.start(intent, this)
        finish()
    }

    companion object {

        var DONE = false
    }
}
