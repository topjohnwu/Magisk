package com.topjohnwu.magisk

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.SharedPreferences
import android.content.res.Configuration
import android.os.AsyncTask
import android.os.Build
import android.os.Bundle
import android.preference.PreferenceManager
import androidx.appcompat.app.AppCompatDelegate
import com.topjohnwu.magisk.data.database.MagiskDB
import com.topjohnwu.magisk.data.database.RepoDatabaseHelper
import com.topjohnwu.magisk.di.koinModules
import com.topjohnwu.magisk.utils.LocaleManager
import com.topjohnwu.magisk.utils.RootUtils
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin
import java.util.concurrent.ThreadPoolExecutor

open class App : Application(), Application.ActivityLifecycleCallbacks {

    // Global resources
    val prefs: SharedPreferences by lazy { PreferenceManager.getDefaultSharedPreferences(deContext) }
    val DB: MagiskDB by lazy { MagiskDB(deContext) }
    @JvmField
    var repoDB: RepoDatabaseHelper? = null
    @Volatile
    private var foreground: Activity? = null

    override fun onCreate() {
        super.onCreate()

        startKoin {
            androidContext(this@App)
            modules(koinModules)
        }
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base)
        self = this
        deContext = base
        registerActivityLifecycleCallbacks(this)

        if (Build.VERSION.SDK_INT >= 24) {
            deContext = base.createDeviceProtectedStorageContext()
            deContext.moveSharedPreferencesFrom(
                base,
                PreferenceManager.getDefaultSharedPreferencesName(base)
            )
        }

        Networking.init(base)
        LocaleManager.setLocale(this)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        LocaleManager.setLocale(this)
    }

    //region ActivityLifecycleCallbacks
    override fun onActivityCreated(activity: Activity, bundle: Bundle?) {}

    override fun onActivityStarted(activity: Activity) {}

    @Synchronized
    override fun onActivityResumed(activity: Activity) {
        foreground = activity
    }

    @Synchronized
    override fun onActivityPaused(activity: Activity) {
        foreground = null
    }

    override fun onActivityStopped(activity: Activity) {}

    override fun onActivitySaveInstanceState(activity: Activity, bundle: Bundle) {}

    override fun onActivityDestroyed(activity: Activity) {}
    //endregion

    companion object {

        //fixme this should be at least weak reference, me no likey
        @SuppressLint("StaticFieldLeak")
        @JvmStatic
        lateinit var self: App

        //fixme this should be at least weak reference, me no likey
        @SuppressLint("StaticFieldLeak")
        @JvmStatic
        lateinit var deContext: Context

        //fixme me no likey
        @JvmField
        var THREAD_POOL: ThreadPoolExecutor

        init {
            AppCompatDelegate.setCompatVectorFromResourcesEnabled(true)
            Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER or Shell.FLAG_USE_MAGISK_BUSYBOX)
            Shell.Config.verboseLogging(BuildConfig.DEBUG)
            Shell.Config.addInitializers(RootUtils::class.java)
            Shell.Config.setTimeout(2)
            THREAD_POOL = AsyncTask.THREAD_POOL_EXECUTOR as ThreadPoolExecutor
        }

        //fixme me no likey
        @JvmStatic
        fun foreground(): Activity? {
            return self.foreground
        }
    }
}
