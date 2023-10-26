package com.topjohnwu.magisk.core

import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.res.Configuration
import android.os.Bundle
import androidx.profileinstaller.ProfileInstaller
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.utils.DispatcherExecutor
import com.topjohnwu.magisk.core.utils.NetworkObserver
import com.topjohnwu.magisk.core.utils.ProcessLifecycle
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.core.utils.ShellInit
import com.topjohnwu.magisk.core.utils.refreshLocale
import com.topjohnwu.magisk.core.utils.setConfig
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import timber.log.Timber
import java.lang.ref.WeakReference
import kotlin.system.exitProcess

open class App() : Application() {

    constructor(o: Any) : this() {
        val data = StubApk.Data(o)
        // Add the root service name mapping
        data.classToComponent[RootUtils::class.java.name] = data.rootService.name
        // Send back the actual root service class
        data.rootService = RootUtils::class.java
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
        // Get the actual ContextImpl
        val app: Application
        val base: Context
        if (context is Application) {
            app = context
            base = context.baseContext
            AppApkPath = StubApk.current(base).path
        } else {
            app = this
            base = context
            AppApkPath = base.packageResourcePath
        }
        super.attachBaseContext(base)
        ServiceLocator.context = base
        app.registerActivityLifecycleCallbacks(ActivityTracker)

        Shell.setDefaultBuilder(Shell.Builder.create()
            .setFlags(Shell.FLAG_MOUNT_MASTER)
            .setInitializers(ShellInit::class.java)
            .setContext(base)
            .setTimeout(2))
        Shell.EXECUTOR = DispatcherExecutor(Dispatchers.IO)
        RootUtils.bindTask = RootService.bindOrTask(
            intent<RootUtils>(),
            UiThreadHandler.executor,
            RootUtils.Connection
        )
        // Pre-heat the shell ASAP
        Shell.getShell(null) {}

        refreshLocale()
        resources.patch()
        Notifications.setup()
    }

    override fun onCreate() {
        super.onCreate()
        ProcessLifecycle.init(this)
        NetworkObserver.init(this)
        if (!BuildConfig.DEBUG && !isRunningAsStub) {
            GlobalScope.launch(Dispatchers.IO) {
                ProfileInstaller.writeProfile(this@App)
            }
        }
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        if (resources.configuration.diff(newConfig) != 0) {
            resources.setConfig(newConfig)
        }
        if (!isRunningAsStub)
            super.onConfigurationChanged(newConfig)
    }
}

object ActivityTracker : Application.ActivityLifecycleCallbacks {

    val foreground: Activity? get() = ref.get()

    @Volatile
    private var ref = WeakReference<Activity>(null)

    override fun onActivityResumed(activity: Activity) {
        if (activity is SuRequestActivity) return
        ref = WeakReference(activity)
    }

    override fun onActivityPaused(activity: Activity) {
        if (activity is SuRequestActivity) return
        ref.clear()
    }

    override fun onActivityCreated(activity: Activity, bundle: Bundle?) {}
    override fun onActivityStarted(activity: Activity) {}
    override fun onActivityStopped(activity: Activity) {}
    override fun onActivitySaveInstanceState(activity: Activity, bundle: Bundle) {}
    override fun onActivityDestroyed(activity: Activity) {}
}
