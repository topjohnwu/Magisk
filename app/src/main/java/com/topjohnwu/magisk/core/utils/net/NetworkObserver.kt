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
            return when (Build.VERSION.SDK_INT) {
                in 23 until Int.MAX_VALUE -> MarshmallowNetworkObserver(context, callback)
                in 21 until 23 -> LollipopNetworkObserver(context, callback)
                else -> PreLollipopNetworkObserver(context, callback)
            }.apply { getCurrentState() }
        }
    }
}
