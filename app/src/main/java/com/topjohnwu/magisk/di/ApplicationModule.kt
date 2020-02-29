package com.topjohnwu.magisk.di

import android.annotation.SuppressLint
import android.app.Activity
import android.app.Application
import android.content.Context
import android.os.Build
import android.os.Bundle
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import androidx.preference.PreferenceManager
import com.topjohnwu.magisk.core.ResMgr
import com.topjohnwu.magisk.utils.RxBus
import org.koin.core.qualifier.named
import org.koin.dsl.module

val SUTimeout = named("su_timeout")
val Protected = named("protected")

val applicationModule = module {
    single { RxBus() }
    factory { ResMgr.resource }
    factory { get<Context>().packageManager }
    factory(Protected) { createDEContext(get()) }
    single(SUTimeout) { get<Context>(Protected).getSharedPreferences("su_timeout", 0) }
    single { PreferenceManager.getDefaultSharedPreferences(get<Context>(Protected)) }
    single { ActivityTracker() }
    factory { get<ActivityTracker>().foreground ?: NullActivity }
    single { LocalBroadcastManager.getInstance(get()) }
}

private fun createDEContext(context: Context): Context {
    return if (Build.VERSION.SDK_INT >= 24)
        context.createDeviceProtectedStorageContext()
    else context
}

class ActivityTracker : Application.ActivityLifecycleCallbacks {

    @Volatile
    var foreground: Activity? = null

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
}

@SuppressLint("Registered")
object NullActivity : Activity()
