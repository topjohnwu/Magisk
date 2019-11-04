package com.topjohnwu.magisk.ui

import android.app.Activity
import android.content.Context
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.intent
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.magisk.wrap
import com.topjohnwu.superuser.Shell

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

        // Set default configs
        Config.initialize()

        // Create notification channel on Android O
        Notifications.setup(this)

        // Schedule periodic update checks
        Utils.scheduleUpdateCheck(this)

        // Setup shortcuts
        Shortcuts.setup(this)

        Shell.su("mm_patch_dtbo").submit {
            if (it.isSuccess)
                Notifications.dtboPatched(this)
        }

        DONE = true

        startActivity(intent<MainActivity>().apply { intent?.also { putExtras(it) } })
        finish()
    }

    companion object {

        var DONE = false
    }
}
