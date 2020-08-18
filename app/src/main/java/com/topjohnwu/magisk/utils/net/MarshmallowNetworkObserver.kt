@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.utils.net

import android.annotation.TargetApi
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.NetworkInfo
import android.os.PowerManager
import androidx.core.content.getSystemService

@TargetApi(23)
class MarshmallowNetworkObserver(
    context: Context,
    callback: NetInfoCallback
): LollipopNetworkObserver(context, callback) {

    private val idleReceiver = IdleBroadcastReceiver()

    init {
        val filter = IntentFilter(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED)
        context.registerReceiver(idleReceiver, filter)
    }

    override fun stopObserving() {
        super.stopObserving()
        context.unregisterReceiver(idleReceiver)
    }

    override fun emit(current: Connectivity) {
        val typeChanged = last.type != current.type
        val wasConnected = last.state == NetworkInfo.State.CONNECTED
        val isDisconnected = current.state == NetworkInfo.State.DISCONNECTED
        val isNotIdle = current.detailedState != NetworkInfo.DetailedState.IDLE
        if (typeChanged && wasConnected && isDisconnected && isNotIdle) {
            super.emit(current)
            super.emit(last)
            last = current
        } else {
            super.emit(current)
        }
    }

    private inner class IdleBroadcastReceiver: BroadcastReceiver() {

        private fun isIdleMode(context: Context): Boolean {
            val packageName = context.packageName
            val manager = context.getSystemService<PowerManager>()!!
            val isIgnoringOptimizations = manager.isIgnoringBatteryOptimizations(packageName)
            return manager.isDeviceIdleMode && !isIgnoringOptimizations
        }

        override fun onReceive(context: Context, intent: Intent) {
            if (isIdleMode(context)) {
                emit(Connectivity())
            } else {
                emit(Connectivity.create(manager))
            }
        }
    }
}
