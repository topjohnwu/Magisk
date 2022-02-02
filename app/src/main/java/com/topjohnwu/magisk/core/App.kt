package com.topjohnwu.magisk.core

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.res.Configuration
import android.os.Bundle
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.utils.*
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import timber.log.Timber
import kotlin.system.exitProcess

open class App() : Application() {

    constructor(o: Any) : this() {
        val data = StubApk.Data(o)
        // Add the root service name mapping
        data.classToComponent[RootRegistry::class.java.name] = data.rootService.name
        // Send back the actual root service class
        data.rootService = RootRegistry::class.java
        Info.stub = data
    }

    init {
        // Always log full stack trace with Timber
        Timber.plant(Timber.DebugTree())
        Thread.setDefaultUncaughtExceptionHandler { _, e ->
            Timber.e(e)
            exitProcess(1)
        }
    }

    override fun attachBaseContext(context: Context) {
        Shell.setDefaultBuilder(Shell.Builder.create()
            .setFlags(Shell.FLAG_MOUNT_MASTER)
            .setInitializers(ShellInit::class.java)
            .setTimeout(2))
        Shell.EXECUTOR = DispatcherExecutor(Dispatchers.IO)

        // Get the actual ContextImpl
        val app: Application
        val base: Context
        if (context is Application) {
            app = context
            base = context.baseContext
        } else {
            app = this
            base = context
        }
        super.attachBaseContext(base)
        ServiceLocator.context = base

        refreshLocale()
        AppApkPath = if (isRunningAsStub) {
            StubApk.current(base).path
        } else {
            base.packageResourcePath
        }

        base.resources.patch()
        app.registerActivityLifecycleCallbacks(ActivityTracker)
    }

    override fun onCreate() {
        super.onCreate()
        RootRegistry.bindTask = RootService.createBindTask(
            intent<RootRegistry>(),
            UiThreadHandler.executor,
            RootRegistry.Connection
        )
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        if (resources.configuration.diff(newConfig) != 0) {
            resources.setConfig(newConfig)
        }
        if (!isRunningAsStub)
            super.onConfigurationChanged(newConfig)
    }
}

@SuppressLint("StaticFieldLeak")
object ActivityTracker : Application.ActivityLifecycleCallbacks {

    @Volatile
    var foreground: Activity? = null

    val hasForeground get() = foreground != null

    override fun onActivityResumed(activity: Activity) {
        foreground = activity
    }

    override fun onActivityPaused(activity: Activity) {
        foreground = null
    }

    override fun onActivityCreated(activity: Activity, bundle: Bundle?) {}
    override fun onActivityStarted(activity: Activity) {}
    override fun onActivityStopped(activity: Activity) {}
    override fun onActivitySaveInstanceState(activity: Activity, bundle: Bundle) {}
    override fun onActivityDestroyed(activity: Activity) {}
}
