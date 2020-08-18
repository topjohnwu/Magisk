@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.utils.net

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager

class PreLollipopNetworkObserver(
    context: Context,
    callback: NetInfoCallback
): NetworkObserver(context, callback) {

    private val receiver = ConnectivityBroadcastReceiver()

    init {
        val filter = IntentFilter()
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION)
        context.registerReceiver(receiver, filter)
    }

    override fun stopObserving() {
        context.unregisterReceiver(receiver)
    }

    private inner class ConnectivityBroadcastReceiver: BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            emit(Connectivity.create(manager))
        }
    }

}
