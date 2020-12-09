package com.topjohnwu.magisk.core

import android.app.Activity
import android.content.Context
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.NetworkService
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

    private fun handleRepackage(pkg: String?) {
        if (packageName != APPLICATION_ID) {
            runCatching {
                // Hidden, remove com.topjohnwu.magisk if exist as it could be malware
                packageManager.getApplicationInfo(APPLICATION_ID, 0)
                Shell.su("(pm uninstall $APPLICATION_ID)& >/dev/null 2>&1").exec()
            }
        } else {
            if (Config.suManager.isNotEmpty())
                Config.suManager = ""
            pkg ?: return
            Shell.su("(pm uninstall $pkg)& >/dev/null 2>&1").exec()
        }
    }

    private fun initAndStart() {
        // Pre-initialize root shell
        Shell.getShell()

        val prevPkg = intent.getStringExtra(Const.Key.PREV_PKG)

        Config.load(prevPkg)
        handleRepackage(prevPkg)
        Notifications.setup(this)
        UpdateCheckService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network services
        get<NetworkService>()

        DONE = true

        redirect<MainActivity>().also { startActivity(it) }
        finish()
    }

    companion object {
        var DONE = false
    }
}
