package com.topjohnwu.magisk.arch

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.result.contract.ActivityResultContract
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BuildConfig.APPLICATION_ID
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.*
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import java.util.concurrent.CountDownLatch

abstract class BaseMainActivity<Binding : ViewDataBinding> : NavigationActivity<Binding>() {

    companion object {
        private var doPreload = true
    }

    private val latch = CountDownLatch(1)
    private val uninstallPkg = registerForActivityResult(UninstallPackage) { latch.countDown() }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(Theme.selected.themeRes)

        if (isRunningAsStub && doPreload) {
            // Manually apply splash theme for stub
            theme.applyStyle(R.style.StubSplashTheme, true)
        }

        super.onCreate(savedInstanceState)

        if (!isRunningAsStub) {
            val splashScreen = installSplashScreen()
            splashScreen.setKeepOnScreenCondition { doPreload }
        }

        if (doPreload) {
            Shell.getShell(null) {
                if (isRunningAsStub && !it.isRoot) {
                    showInvalidStateMessage()
                    return@getShell
                }
                preLoad()
                runOnUiThread {
                    doPreload = false
                    if (isRunningAsStub) {
                        // Re-launch main activity without splash theme
                        relaunch()
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
            MagiskDialog(this).apply {
                setTitle(R.string.unsupport_nonroot_stub_title)
                setMessage(R.string.unsupport_nonroot_stub_msg)
                setButton(MagiskDialog.ButtonType.POSITIVE) {
                    text = R.string.install
                    onClick { HideAPK.restore(this@BaseMainActivity) }
                }
                setCancelable(false)
                show()
            }
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
            if (!Shell.su("(pm uninstall $pkg)& >/dev/null 2>&1").exec().isSuccess) {
                uninstallPkg.launch(pkg)
                // Wait for the uninstallation to finish
                latch.await()
            }
        }
    }

    object UninstallPackage : ActivityResultContract<String, Boolean>() {

        @Suppress("DEPRECATION")
        override fun createIntent(context: Context, input: String): Intent {
            val uri = Uri.Builder().scheme("package").opaquePart(input).build()
            val intent = Intent(Intent.ACTION_UNINSTALL_PACKAGE, uri)
            intent.putExtra(Intent.EXTRA_RETURN_RESULT, true)
            return intent
        }

        override fun parseResult(resultCode: Int, intent: Intent?) = resultCode == RESULT_OK
    }
}
