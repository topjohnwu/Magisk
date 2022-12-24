package com.topjohnwu.magisk.core.utils.net

import android.content.Context
import android.net.ConnectivityManager
import android.os.Build
import androidx.core.content.getSystemService

typealias ConnectionCallback = (Boolean) -> Unit

abstract class NetworkObserver(
    context: Context,
    protected val callback: ConnectionCallback
) {

    protected val app: Context = context.applicationContext
    protected val manager = context.getSystemService<ConnectivityManager>()!!

    protected abstract fun stopObserving()
    protected abstract fun getCurrentState()

    companion object {
        fun observe(context: Context, callback: ConnectionCallback): NetworkObserver {
            val observer: NetworkObserver = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
                MarshmallowNetworkObserver(context, callback)
            else LollipopNetworkObserver(context, callback)
            return observer.apply { getCurrentState() }
        }
    }
}
