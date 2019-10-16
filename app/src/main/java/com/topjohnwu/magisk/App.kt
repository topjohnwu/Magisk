package com.topjohnwu.magisk

import android.app.Application
import android.content.Context
import android.content.res.Configuration
import androidx.appcompat.app.AppCompatDelegate
import androidx.multidex.MultiDex
import androidx.room.Room
import androidx.work.impl.WorkDatabase
import androidx.work.impl.WorkDatabase_Impl
import com.topjohnwu.magisk.data.database.RepoDatabase
import com.topjohnwu.magisk.data.database.RepoDatabase_Impl
import com.topjohnwu.magisk.di.ActivityTracker
import com.topjohnwu.magisk.di.koinModules
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.unwrap
import com.topjohnwu.magisk.utils.ResourceMgr
import com.topjohnwu.magisk.utils.RootInit
import com.topjohnwu.magisk.utils.isRunningAsStub
import com.topjohnwu.magisk.utils.wrap
import com.topjohnwu.superuser.Shell
import org.koin.android.ext.koin.androidContext
import org.koin.core.context.startKoin
import timber.log.Timber

open class App : Application() {

    init {
        AppCompatDelegate.setCompatVectorFromResourcesEnabled(true)
        Shell.Config.setFlags(Shell.FLAG_MOUNT_MASTER or Shell.FLAG_USE_MAGISK_BUSYBOX)
        Shell.Config.verboseLogging(BuildConfig.DEBUG)
        Shell.Config.addInitializers(RootInit::class.java)
        Shell.Config.setTimeout(2)
        Room.setFactory {
            when (it) {
                WorkDatabase::class.java -> WorkDatabase_Impl()
                RepoDatabase::class.java -> RepoDatabase_Impl()
                else -> null
            }
        }
    }

    override fun attachBaseContext(base: Context) {
        // Basic setup
        if (BuildConfig.DEBUG)
            MultiDex.install(base)
        Timber.plant(Timber.DebugTree())

        // Some context magic
        val app: Application
        val impl: Context
        if (base is Application) {
            isRunningAsStub = true
            app = base
            impl = base.baseContext
        } else {
            app = this
            impl = base
        }
        ResourceMgr.init(impl)
        super.attachBaseContext(impl.wrap())

        // Normal startup
        startKoin {
            androidContext(baseContext)
            modules(koinModules)
        }
        ResourceMgr.reload()
        app.registerActivityLifecycleCallbacks(get<ActivityTracker>())
    }

    // This is required as some platforms expect ContextImpl
    override fun getBaseContext(): Context {
        return super.getBaseContext().unwrap()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        ResourceMgr.reload(newConfig)
        if (!isRunningAsStub)
            super.onConfigurationChanged(newConfig)
    }
}
