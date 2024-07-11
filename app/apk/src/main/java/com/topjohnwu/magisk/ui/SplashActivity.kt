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
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.core.base.relaunch
import com.topjohnwu.magisk.core.initializeOnSplashScreen
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch
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
                initializeOnSplashScreen {
                    splashShown = true
                    if (isRunningAsStub) {
                        // Re-launch main activity without splash theme
                        relaunch()
                    } else {
                        if (lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)) {
                            doShowMainUI(savedInstanceState)
                        } else {
                            needShowMainUI = true
                        }
                    }
                }
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
}
