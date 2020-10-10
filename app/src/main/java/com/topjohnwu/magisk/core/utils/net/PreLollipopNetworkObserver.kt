@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core.utils.net

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import androidx.core.net.ConnectivityManagerCompat

class PreLollipopNetworkObserver(
    context: Context,
    callback: ConnectionCallback
): NetworkObserver(context, callback) {

    private val receiver = ConnectivityBroadcastReceiver()

    init {
        val filter = IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION)
        app.registerReceiver(receiver, filter)
    }

    override fun stopObserving() {
        app.unregisterReceiver(receiver)
    }

    override fun getCurrentState() {
        callback(manager.activeNetworkInfo?.isConnected ?: false)
    }

    private inner class ConnectivityBroadcastReceiver: BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent) {
            val info = ConnectivityManagerCompat.getNetworkInfoFromBroadcast(manager, intent)
            callback(info?.isConnected ?: false)
        }
    }
}
