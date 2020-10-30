package com.topjohnwu.magisk.core

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import java.util.concurrent.CountDownLatch

open class SplashActivity : Activity() {

    private val latch = CountDownLatch(1)

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(R.style.SplashTheme)
        super.onCreate(savedInstanceState)
        // Pre-initialize root shell
        Shell.getShell(null) { initAndStart() }
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
            val uninstall = Shell.su("(pm uninstall $pkg)& >/dev/null 2>&1").exec()
            if (!uninstall.isSuccess) uninstallApp(pkg)
        }
    }

    private fun initAndStart() {
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

    @Suppress("DEPRECATION")
    private fun uninstallApp(pkg: String) {
        val uri = Uri.Builder().scheme("package").opaquePart(pkg).build()
        val intent = Intent(Intent.ACTION_UNINSTALL_PACKAGE, uri)
        intent.putExtra(Intent.EXTRA_RETURN_RESULT, true)
        startActivityForResult(intent, Const.ID.UNINSTALL_APP)
        latch.await()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == Const.ID.UNINSTALL_APP) {
            if (resultCode != RESULT_CANCELED) latch.countDown()
        } else super.onActivityResult(requestCode, resultCode, data)
    }

    companion object {
        var DONE = false
    }
}
