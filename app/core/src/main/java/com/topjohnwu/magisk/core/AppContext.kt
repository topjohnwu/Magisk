package com.topjohnwu.magisk.core

import android.app.Activity
import android.app.Application
import android.app.LocaleManager
import android.content.ComponentCallbacks2
import android.content.Context
import android.content.ContextWrapper
import android.content.res.Configuration
import android.os.Build
import android.os.Build.VERSION.SDK_INT
import android.os.Bundle
import android.system.Os
import androidx.profileinstaller.ProfileInstaller
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.base.UntrackedActivity
import com.topjohnwu.magisk.core.utils.LocaleSetting
import com.topjohnwu.magisk.core.utils.NetworkObserver
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.core.utils.ShellInit
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.ipc.RootService
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.asExecutor
import kotlinx.coroutines.launch
import timber.log.Timber
import java.lang.ref.WeakReference
import kotlin.system.exitProcess

lateinit var AppApkPath: String
    private set

object AppContext : ContextWrapper(null),
    Application.ActivityLifecycleCallbacks, ComponentCallbacks2 {

    val foregroundActivity: Activity? get() = ref.get()

    private var ref = WeakReference<Activity>(null)
    private lateinit var application: Application
    private lateinit var networkObserver: NetworkObserver

    init {
        // Always log full stack trace with Timber
        Timber.plant(Timber.DebugTree())
        Thread.setDefaultUncaughtExceptionHandler { _, e ->
            Timber.e(e)
            exitProcess(1)
        }

        Os.setenv("PATH", "${Os.getenv("PATH")}:/debug_ramdisk:/sbin", true)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        LocaleSetting.instance.updateResource(resources)
    }

    override fun onActivityStarted(activity: Activity) {
        networkObserver.postCurrentState()
    }

    override fun onActivityResumed(activity: Activity) {
        if (activity is UntrackedActivity) return
        ref = WeakReference(activity)
    }

    override fun onActivityPaused(activity: Activity) {
        if (activity is UntrackedActivity) return
        ref.clear()
    }

    override fun getApplicationContext() = application

    fun attachApplication(app: Application) {
        application = app
        val base = app.baseContext
        attachBaseContext(base)
        app.registerActivityLifecycleCallbacks(this)
        app.registerComponentCallbacks(this)

        AppApkPath = if (isRunningAsStub) {
            StubApk.current(base).path
        } else {
            base.packageResourcePath
        }
        resources.patch()

        val shellBuilder = Shell.Builder.create()
            .setFlags(Shell.FLAG_MOUNT_MASTER)
            .setInitializers(ShellInit::class.java)
            .setContext(this)
            .setTimeout(2)
        Shell.setDefaultBuilder(shellBuilder)
        Shell.EXECUTOR = Dispatchers.IO.asExecutor()
        RootUtils.bindTask = RootService.bindOrTask(
            intent<RootUtils>(),
            UiThreadHandler.executor,
            RootUtils.Connection
        )
        // Pre-heat the shell ASAP
        Shell.getShell(null) {}

        if (SDK_INT >= 34 && isRunningAsStub) {
            // Send over the locale config manually
            val lm = getSystemService(LocaleManager::class.java)
            lm.overrideLocaleConfig = LocaleSetting.localeConfig
        }
        networkObserver = NetworkObserver.init(this)
        if (!BuildConfig.DEBUG && !isRunningAsStub) {
            GlobalScope.launch(Dispatchers.IO) {
                ProfileInstaller.writeProfile(this@AppContext)
            }
        }
    }

    override fun createDeviceProtectedStorageContext(): Context {
        return if (SDK_INT >= Build.VERSION_CODES.N) {
            super.createDeviceProtectedStorageContext()
        } else {
            this
        }
    }

    override fun onActivityCreated(activity: Activity, bundle: Bundle?) {}
    override fun onActivityStopped(activity: Activity) {}
    override fun onActivitySaveInstanceState(activity: Activity, bundle: Bundle) {}
    override fun onActivityDestroyed(activity: Activity) {}
    override fun onLowMemory() {}
    override fun onTrimMemory(level: Int) {}
}
