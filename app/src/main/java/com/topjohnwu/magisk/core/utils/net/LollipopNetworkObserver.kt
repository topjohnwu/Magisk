package com.topjohnwu.magisk.core.utils.net

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.os.Build
import androidx.collection.ArraySet

open class LollipopNetworkObserver(
    context: Context,
    callback: ConnectionCallback
): NetworkObserver(context, callback) {

    private val networkCallback = NetCallback()

    init {
        val builder = NetworkRequest.Builder()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            builder.addCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)
        } else {
            builder.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
        }
        manager.registerNetworkCallback(builder.build(), networkCallback)
    }

    @Suppress("DEPRECATION")
    override fun postCurrentState() {
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
