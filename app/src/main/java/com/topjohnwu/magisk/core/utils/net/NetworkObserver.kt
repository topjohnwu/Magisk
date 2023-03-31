package com.topjohnwu.magisk.core.utils.net

import android.content.Context
import android.net.ConnectivityManager
import android.os.Build
import androidx.core.content.getSystemService
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.ProcessLifecycleOwner

typealias ConnectionCallback = (Boolean) -> Unit

abstract class NetworkObserver(
    context: Context,
    protected val callback: ConnectionCallback
): DefaultLifecycleObserver {

    protected val app: Context = context.applicationContext
    protected val manager = context.getSystemService<ConnectivityManager>()!!

    init {
        ProcessLifecycleOwner.get().lifecycle.addObserver(this)
    }

    protected abstract fun stopObserving()
    protected abstract fun postCurrentState()

    override fun onStart(owner: LifecycleOwner) {
        postCurrentState()
    }

    companion object {
        fun observe(context: Context, callback: ConnectionCallback): NetworkObserver {
            val observer: NetworkObserver = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
                MarshmallowNetworkObserver(context, callback)
            else LollipopNetworkObserver(context, callback)
            return observer.apply { postCurrentState() }
        }
    }
}
