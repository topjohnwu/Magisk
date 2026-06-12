package com.topjohnwu.magisk.core.utils

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.os.PowerManager
import androidx.collection.ArraySet
import androidx.core.content.getSystemService
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.registerRuntimeReceiver

class NetworkObserver(context: Context) {
    private val manager = context.getSystemService<ConnectivityManager>()!!

    private val networkCallback = object : ConnectivityManager.NetworkCallback() {
        private val activeList = ArraySet<Network>()

        override fun onAvailable(network: Network) {
            activeList.add(network)
            postValue(true)
        }
        override fun onLost(network: Network) {
            activeList.remove(network)
            postValue(!activeList.isEmpty())
        }
    }

    private val receiver = object : BroadcastReceiver() {
        private fun Context.isIdleMode(): Boolean {
            val pwm = getSystemService<PowerManager>() ?: return true
            val isIgnoringOptimizations = pwm.isIgnoringBatteryOptimizations(packageName)
            return pwm.isDeviceIdleMode && !isIgnoringOptimizations
        }
        override fun onReceive(context: Context, intent: Intent) {
            if (context.isIdleMode()) {
                postValue(false)
            } else {
                postCurrentState()
            }
        }
    }

    init {
        val request = NetworkRequest.Builder()
            .addCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)
            .build()
        manager.registerNetworkCallback(request, networkCallback)
        val filter = IntentFilter(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED)
        context.applicationContext.registerRuntimeReceiver(receiver, filter)
    }

    fun postCurrentState() {
        postValue(
            manager.getNetworkCapabilities(manager.activeNetwork)
                ?.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED) == true
        )
    }

    private fun postValue(b: Boolean) {
        Info.resetUpdate()
        Info.isConnected.postValue(b)
    }

    companion object {
        fun init(context: Context): NetworkObserver {
            return NetworkObserver(context).apply { postCurrentState() }
        }
    }
}
