package com.topjohnwu.magisk.core

import android.app.Activity
import android.content.Context
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

open class SplashActivity : Activity() {

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(R.style.SplashTheme)
        super.onCreate(savedInstanceState)
        GlobalScope.launch(Dispatchers.IO) {
            initAndStart()
        }
    }

    private fun handleRepackage() {
        val pkg = Config.suManager
        if (Config.suManager.isNotEmpty() && packageName == BuildConfig.APPLICATION_ID) {
            Config.suManager = ""
            Shell.su("(pm uninstall $pkg)& >/dev/null 2>&1").exec()
        }
        if (pkg == packageName) {
            runCatching {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                packageManager.getApplicationInfo(BuildConfig.APPLICATION_ID, 0)
                Shell.su("(pm uninstall ${BuildConfig.APPLICATION_ID})& >/dev/null 2>&1").exec()
            }
        }
    }

    private fun initAndStart() {
        // Pre-initialize root shell
        Shell.getShell()

        Config.initialize()
        handleRepackage()
        Notifications.setup(this)
        UpdateCheckService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network stuffs
        get<GithubRawServices>()

        DONE = true

        redirect<MainActivity>().also { startActivity(it) }
        finish()
    }

    companion object {
        var DONE = false
    }
}
