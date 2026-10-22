package com.topjohnwu.magisk.core.base

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.lifecycle.Lifecycle
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.JobService
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Shortcuts
import com.topjohnwu.superuser.Shell

interface SplashScreenHost : IActivityExtension {
    val splashController: SplashController<*>

    fun onCreateUi(savedInstanceState: Bundle?)
}

class SplashController<T>(private val activity: T)
    where T: ComponentActivity, T: SplashScreenHost {

    companion object {
        private var splashShown = false
    }

    private var shouldCreateUiOnResume = false

    fun preOnCreate() {}

    fun onCreate(savedInstanceState: Bundle?) {
        val splashScreen = activity.installSplashScreen()
        splashScreen.setKeepOnScreenCondition { !splashShown }

        if (splashShown) {
            doCreateUi(savedInstanceState)
        } else {
            Shell.getShell(Shell.EXECUTOR) {
                activity.initializeApp()
                activity.runOnUiThread {
                    splashShown = true
                    if (activity.lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)) {
                        doCreateUi(savedInstanceState)
                    } else {
                        shouldCreateUiOnResume = true
                    }
                }
            }
        }
    }

    fun onResume() {
        if (shouldCreateUiOnResume) {
            doCreateUi(null)
        }
    }

    private fun doCreateUi(savedInstanceState: Bundle?) {
        shouldCreateUiOnResume = false
        activity.onCreateUi(savedInstanceState)
    }

    private fun T.initializeApp() {
        Config.init()

        Notifications.setup()
        JobService.schedule(this)
        Shortcuts.setupDynamic(this)

        // Pre-fetch network services
        ServiceLocator.networkService

        // Wait for root service
        RootUtils.Connection.await()
    }
}
