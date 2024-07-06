package com.topjohnwu.magisk.ui

import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.annotation.SuppressLint
import android.os.Bundle
import android.widget.Toast
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.databinding.ViewDataBinding
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.JobService
import com.topjohnwu.magisk.core.base.realCallingPackage
import com.topjohnwu.magisk.core.base.relaunch
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.File
import java.io.IOException
import com.topjohnwu.magisk.core.R as CoreR

@SuppressLint("CustomSplashScreen")
abstract class SplashActivity<Binding : ViewDataBinding> : NavigationActivity<Binding>() {

    companion object {
        private var splashShown = false
    }

    private var needShowMainUI = false

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(Theme.selected.themeRes)

        if (isRunningAsStub && !splashShown) {
            // Manually apply splash theme for stub
            theme.applyStyle(R.style.StubSplashTheme, true)
        }

        super.onCreate(savedInstanceState)

        if (!isRunningAsStub) {
            val splashScreen = installSplashScreen()
            splashScreen.setKeepOnScreenCondition { !splashShown }
        }

        if (splashShown) {
            doShowMainUI(savedInstanceState)
        } else {
            Shell.getShell(Shell.EXECUTOR) {
                if (isRunningAsStub && !it.isRoot) {
                    showInvalidStateMessage()
                    return@getShell
                }
                initialize(savedInstanceState)
            }
        }
    }

    private fun doShowMainUI(savedInstanceState: Bundle?) {
        needShowMainUI = false
        showMainUI(savedInstanceState)
    }

    abstract fun showMainUI(savedInstanceState: Bundle?)

    @SuppressLint("InlinedApi")
    private fun showInvalidStateMessage(): Unit = runOnUiThread {
        MagiskDialog(this).apply {
            setTitle(CoreR.string.unsupport_nonroot_stub_title)
            setMessage(CoreR.string.unsupport_nonroot_stub_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = CoreR.string.install
                onClick {
                    withPermission(REQUEST_INSTALL_PACKAGES) {
                        if (!it) {
                            toast(CoreR.string.install_unknown_denied, Toast.LENGTH_SHORT)
                            showInvalidStateMessage()
                        } else {
                            lifecycleScope.launch {
                                HideAPK.restore(this@SplashActivity)
                            }
                        }
                    }
                }
            }
            setCancelable(false)
            show()
        }
    }

    override fun onResume() {
        super.onResume()
        if (needShowMainUI) {
            doShowMainUI(null)
        }
    }

    private fun initialize(savedState: Bundle?) {
        val prevPkg = intent.getStringExtra(Const.Key.PREV_PKG)?.let {
            // Make sure the calling package matches (prevent DoS)
            if (it == realCallingPackage)
                it
            else
                null
        }

        Config.load(prevPkg)

        if (packageName != APP_PACKAGE_NAME) {
            runCatching {
                // Hidden, remove com.topjohnwu.magisk if exist as it could be malware
                packageManager.getApplicationInfo(APP_PACKAGE_NAME, 0)
                Shell.cmd("(pm uninstall $APP_PACKAGE_NAME)& >/dev/null 2>&1").exec()
            }
        } else {
            if (Config.suManager.isNotEmpty())
                Config.suManager = ""
            if (prevPkg != null) {
                Shell.cmd("(pm uninstall $prevPkg)& >/dev/null 2>&1").exec()
            }
        }

        if (prevPkg != null) {
            runOnUiThread {
                // Relaunch the process after package migration
                StubApk.restartProcess(this)
            }
            return
        }

        // Validate stub APK
        if (isRunningAsStub && (
                // Version mismatch
                Info.stub!!.version != BuildConfig.STUB_VERSION ||
                // Not properly patched
                intent.component!!.className.contains(HideAPK.PLACEHOLDER)
            )) {
            withPermission(REQUEST_INSTALL_PACKAGES) { granted ->
                if (granted) {
                    lifecycleScope.launch(Dispatchers.IO) {
                        val apk = File(cacheDir, "stub.apk")
                        try {
                            assets.open("stub.apk").writeTo(apk)
                            HideAPK.upgrade(this@SplashActivity, apk)?.let {
                                startActivity(it)
                            }
                        } catch (e: IOException) {
                            Timber.e(e)
                        }
                    }
                }
            }
            return
        }

        JobService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network services
        ServiceLocator.networkService

        // Wait for root service
        RootUtils.Connection.await()

        runOnUiThread {
            splashShown = true
            if (isRunningAsStub) {
                // Re-launch main activity without splash theme
                relaunch()
            } else {
                if (lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)) {
                    doShowMainUI(savedState)
                } else {
                    needShowMainUI = true
                }
            }
        }
    }
}
