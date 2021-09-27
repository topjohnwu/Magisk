package com.topjohnwu.magisk.core

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import java.util.concurrent.CountDownLatch

open class SplashActivity : BaseActivity() {

    private val latch = CountDownLatch(1)

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
            if (!Shell.su("(pm uninstall $pkg)& >/dev/null 2>&1").exec().isSuccess)
                uninstallApp(pkg)
        }
    }

    private fun initAndStart() {
        if (isRunningAsStub && !Shell.rootAccess()) {
            runOnUiThread {
                MagiskDialog(this)
                    .applyTitle(R.string.unsupport_nonroot_stub_title)
                    .applyMessage(R.string.unsupport_nonroot_stub_msg)
                    .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                        titleRes = R.string.install
                        onClick { HideAPK.restore(this@SplashActivity) }
                    }
                    .cancellable(false)
                    .reveal()
            }
            return
        }

        val prevPkg = intent.getStringExtra(Const.Key.PREV_PKG)

        Config.load(prevPkg)
        handleRepackage(prevPkg)
        Notifications.setup(this)
        UpdateCheckService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network services
        ServiceLocator.networkService

        DONE = true
        startActivity(redirect<MainActivity>())
        finish()
    }

    @Suppress("DEPRECATION")
    private fun uninstallApp(pkg: String) {
        val uri = Uri.Builder().scheme("package").opaquePart(pkg).build()
        val intent = Intent(Intent.ACTION_UNINSTALL_PACKAGE, uri)
        intent.putExtra(Intent.EXTRA_RETURN_RESULT, true)
        startActivityForResult(intent) { _, _ ->
            latch.countDown()
        }
        latch.await()
    }

    companion object {
        var DONE = false
    }
}
