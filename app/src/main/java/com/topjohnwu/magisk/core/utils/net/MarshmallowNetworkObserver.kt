@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core.utils.net

import android.annotation.TargetApi
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.NetworkCapabilities
import android.os.PowerManager
import androidx.core.content.getSystemService

@TargetApi(23)
class MarshmallowNetworkObserver(
    context: Context,
    callback: ConnectionCallback
): LollipopNetworkObserver(context, callback) {

    private val receiver = IdleBroadcastReceiver()

    init {
        val filter = IntentFilter(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED)
        app.registerReceiver(receiver, filter)
    }

    override fun stopObserving() {
        super.stopObserving()
        app.unregisterReceiver(receiver)
    }

    override fun getCurrentState() {
        callback(manager.getNetworkCapabilities(manager.activeNetwork)
            ?.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) ?: false)
    }

    private inner class IdleBroadcastReceiver: BroadcastReceiver() {

        private fun Context.isIdleMode(): Boolean {
            val pwm = getSystemService<PowerManager>() ?: return true
            val isIgnoringOptimizations = pwm.isIgnoringBatteryOptimizations(packageName)
            return pwm.isDeviceIdleMode && !isIgnoringOptimizations
        }

        override fun onReceive(context: Context, intent: Intent) {
            if (context.isIdleMode()) {
                callback(false)
            } else {
                getCurrentState()
            }
        }
    }
}
