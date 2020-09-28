package com.topjohnwu.magisk.core.utils.net

import android.annotation.TargetApi
import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import androidx.collection.ArraySet

@TargetApi(21)
open class LollipopNetworkObserver(
    context: Context,
    callback: ConnectionCallback
): NetworkObserver(context, callback) {

    private val networkCallback = NetCallback()

    init {
        val request = NetworkRequest.Builder()
            .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
            .build()
        manager.registerNetworkCallback(request, networkCallback)
    }

    @Suppress("DEPRECATION")
    override fun getCurrentState() {
        callback(manager.activeNetworkInfo?.isConnected ?: false)
    }

    override fun stopObserving() {
        manager.unregisterNetworkCallback(networkCallback)
    }

    private inner class NetCallback : ConnectivityManager.NetworkCallback() {

        private val activeList = ArraySet<Network>()

        override fun onAvailable(network: Network) {
            activeList.add(network)
            callback(true)
        }

        override fun onLost(network: Network) {
            activeList.remove(network)
            callback(!activeList.isEmpty())
        }
    }
}
