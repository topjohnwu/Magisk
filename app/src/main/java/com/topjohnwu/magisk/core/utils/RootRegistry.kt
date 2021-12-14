package com.topjohnwu.magisk.core.utils

import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.Binder
import android.os.IBinder
import com.topjohnwu.superuser.ipc.RootService
import timber.log.Timber
import java.util.concurrent.CountDownLatch
import kotlin.system.exitProcess

class RootRegistry(stub: Any?) : RootService() {

    constructor() : this(null)

    private val className: String? = stub?.javaClass?.name

    init {
        // Always log full stack trace with Timber
        Timber.plant(Timber.DebugTree())
        Thread.setDefaultUncaughtExceptionHandler { _, e ->
            Timber.e(e)
            exitProcess(1)
        }
    }

    override fun onBind(intent: Intent): IBinder {
        // TODO: PLACEHOLDER
        return Binder()
    }

    override fun getComponentName(): ComponentName {
        return ComponentName(packageName, className ?: javaClass.name)
    }

    // TODO: PLACEHOLDER
    object Connection : CountDownLatch(1), ServiceConnection {
        override fun onServiceConnected(name: ComponentName, service: IBinder) {
            Timber.d("onServiceConnected")
            countDown()
        }
        override fun onNullBinding(name: ComponentName) {
            Timber.d("onServiceConnected")
            countDown()
        }
        override fun onServiceDisconnected(name: ComponentName) {
            bind(Intent().setComponent(name), this)
        }
    }

    companion object {
        var bindTask: Runnable? = null
    }
}
