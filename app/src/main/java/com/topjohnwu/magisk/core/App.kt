package com.topjohnwu.magisk.core

import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatDelegate
import androidx.multidex.MultiDex
import androidx.work.WorkManager
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.core.utils.AppShellInit
import com.topjohnwu.magisk.core.utils.BusyBoxInit
import com.topjohnwu.magisk.core.utils.IODispatcherExecutor
import com.topjohnwu.magisk.core.utils.updateConfig
import com.topjohnwu.magisk.di.koinModules
import com.topjohnwu.magisk.ktx.unwrap
import com.topjohnwu.superuser.Shell
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin
import timber.log.Timber
import java.io.File
import kotlin.system.exitProcess

open class App() : Application() {

    constructor(o: Any) : this() {
        Info.stub = DynAPK.load(o)
    }

    init {
        AppCompatDelegate.setCompatVectorFromResourcesEnabled(true)
        Shell.setDefaultBuilder(Shell.Builder.create()
            .setFlags(Shell.FLAG_MOUNT_MASTER)
            .setInitializers(BusyBoxInit::class.java, AppShellInit::class.java)
            .setTimeout(2))
        Shell.EXECUTOR = IODispatcherExecutor()

        // Always log full stack trace with Timber
        Timber.plant(Timber.DebugTree())
        Thread.setDefaultUncaughtExceptionHandler { _, e ->
            Timber.e(e)
            exitProcess(1)
        }
    }

    override fun attachBaseContext(base: Context) {
        // Basic setup
        if (BuildConfig.DEBUG)
            MultiDex.install(base)

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

        val info = base.applicationInfo
        val libDir = runCatching {
            info.javaClass.getDeclaredField("secondaryNativeLibraryDir").get(info) as String?
        }.getOrNull() ?: info.nativeLibraryDir
        Const.NATIVE_LIB_DIR = File(libDir)

        // Normal startup
        startKoin {
            androidContext(wrapped)
            modules(koinModules)
        }
        AssetHack.init(impl)
        app.registerActivityLifecycleCallbacks(ForegroundTracker)
        WorkManager.initialize(impl.wrapJob(), androidx.work.Configuration.Builder().build())
    }

    // This is required as some platforms expect ContextImpl
    override fun getBaseContext(): Context {
        return super.getBaseContext().unwrap()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        resources.updateConfig(newConfig)
        if (!isRunningAsStub)
            super.onConfigurationChanged(newConfig)
    }
}

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
