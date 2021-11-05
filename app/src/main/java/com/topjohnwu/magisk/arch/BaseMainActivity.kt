package com.topjohnwu.magisk.arch

import android.content.Intent
import android.net.Uri
import android.os.Build.VERSION.SDK_INT
import android.os.Bundle
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.JobService
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import java.util.concurrent.CountDownLatch

abstract class BaseMainActivity<VM : BaseViewModel, Binding : ViewDataBinding>
    : BaseUIActivity<VM, Binding>() {

    private val latch = CountDownLatch(1)

    companion object {
        private var doPreload = true
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(Theme.selected.themeRes)

        if (isRunningAsStub && doPreload) {
            // Manually apply splash theme for stub
            if (SDK_INT >= 31) {
                theme.applyStyle(R.style.StubSplashTheme, true)
            } else {
                theme.applyStyle(R.style.SplashTheme, true)
            }
        }

        super.onCreate(savedInstanceState)

        if (!isRunningAsStub) {
            val splashScreen = installSplashScreen()
            splashScreen.setKeepVisibleCondition { doPreload }
        }

        if (doPreload) {
            // Pre-initialize root shell
            Shell.getShell(null) {
                if (isRunningAsStub && !Shell.rootAccess()) {
                    showInvalidStateMessage()
                    return@getShell
                }
                preLoad()
                runOnUiThread {
                    doPreload = false
                    if (isRunningAsStub) {
                        // Re-launch main activity without splash theme
                        recreate()
                    } else {
                        showMainUI(savedInstanceState)
                    }
                }
            }
        } else {
            showMainUI(savedInstanceState)
        }
    }

    abstract fun showMainUI(savedInstanceState: Bundle?)

    private fun showInvalidStateMessage() {
        runOnUiThread {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_nonroot_stub_title)
                .applyMessage(R.string.unsupport_nonroot_stub_msg)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                    titleRes = R.string.install
                    onClick { HideAPK.restore(this@BaseMainActivity) }
                }
                .cancellable(false)
                .reveal()
        }
    }

    private fun preLoad() {
        val prevPkg = intent.getStringExtra(Const.Key.PREV_PKG)

        Config.load(prevPkg)
        handleRepackage(prevPkg)
        Notifications.setup(this)
        JobService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network services
        ServiceLocator.networkService
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
}
