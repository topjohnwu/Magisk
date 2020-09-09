package com.topjohnwu.magisk.utils.net

import android.annotation.TargetApi
import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest

@TargetApi(21)
open class LollipopNetworkObserver(
    context: Context,
    callback: NetInfoCallback
): NetworkObserver(context, callback) {

    private val networkCallback = NetCallback()

    init {
        val request = NetworkRequest.Builder()
            .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
            .build()
        manager.registerNetworkCallback(request, networkCallback)
    }

    override fun stopObserving() {
        manager.unregisterNetworkCallback(networkCallback)
    }

    private inner class NetCallback : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            emit(Connectivity.create(manager, network))
        }

        override fun onLost(network: Network) {
            emit(Connectivity.create(manager, network))
        }
    }
}
