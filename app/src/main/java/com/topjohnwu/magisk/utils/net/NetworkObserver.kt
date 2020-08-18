package com.topjohnwu.magisk.utils.net

import android.content.Context
import android.net.ConnectivityManager
import android.os.Build
import androidx.core.content.getSystemService

typealias NetInfoCallback = (Connectivity) -> Unit

abstract class NetworkObserver protected constructor(
    context: Context,
    private val callback: NetInfoCallback
) {

    protected val context = context.applicationContext
    protected val manager = context.getSystemService<ConnectivityManager>()!!
    protected var last = Connectivity.create(manager)

    init {
        callback(last)
    }

    protected open fun emit(current: Connectivity) {
        if (last != current)
            callback(current)
        last = current
    }

    protected abstract fun stopObserving()

    companion object {
        fun observe(context: Context, callback: NetInfoCallback): NetworkObserver {
            return when (Build.VERSION.SDK_INT) {
                in 23 until Int.MAX_VALUE -> MarshmallowNetworkObserver(context, callback)
                in 21 until 23 -> LollipopNetworkObserver(context, callback)
                else -> PreLollipopNetworkObserver(context, callback)
            }
        }
    }
}
