package com.topjohnwu.magisk.core

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.res.Configuration
import android.os.Bundle
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.core.utils.DispatcherExecutor
import com.topjohnwu.magisk.core.utils.RootRegistry
import com.topjohnwu.magisk.core.utils.ShellInit
import com.topjohnwu.magisk.core.utils.updateConfig
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.unwrap
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import timber.log.Timber
import kotlin.system.exitProcess

open class App() : Application() {

    constructor(o: Any) : this() {
        val data = DynAPK.Data(o)
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

    override fun attachBaseContext(base: Context) {
        Shell.setDefaultBuilder(Shell.Builder.create()
            .setFlags(Shell.FLAG_MOUNT_MASTER)
            .setInitializers(ShellInit::class.java)
            .setTimeout(2))
        Shell.EXECUTOR = DispatcherExecutor(Dispatchers.IO)

        // Some context magic
        val app: Application
        val impl: Context
        if (base is Application) {
            app = base
            impl = base.baseContext
        } else {
            app = this
            impl = base
        }
        val wrapped = impl.wrap()
        super.attachBaseContext(wrapped)

        ServiceLocator.context = wrapped
        AssetHack.init(impl)
        app.registerActivityLifecycleCallbacks(ForegroundTracker)
    }

    override fun onCreate() {
        super.onCreate()
        RootRegistry.bindTask = RootService.createBindTask(
            intent<RootRegistry>(),
            UiThreadHandler.executor,
            RootRegistry.Connection
        )
    }

    // This is required as some platforms expect ContextImpl
    override fun getBaseContext(): Context {
        return super.getBaseContext().unwrap()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        if (resources.configuration.diff(newConfig) != 0) {
            resources.updateConfig(newConfig)
        }
        if (!isRunningAsStub)
            super.onConfigurationChanged(newConfig)
    }
}

@SuppressLint("StaticFieldLeak")
object ForegroundTracker : Application.ActivityLifecycleCallbacks {

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
