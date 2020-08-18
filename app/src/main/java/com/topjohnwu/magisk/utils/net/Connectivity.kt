@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.utils.net

import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkInfo
import android.net.NetworkInfo.DetailedState
import androidx.annotation.RequiresApi

// Recreate NetworkInfo with Kotlin data class
data class Connectivity(
    val state : NetworkInfo.State = NetworkInfo.State.DISCONNECTED,
    val detailedState : DetailedState = DetailedState.IDLE,
    val type : Int = -1,
    val subType : Int = -1,
    val isAvailable : Boolean = false,
    val isFailover : Boolean = false,
    val isRoaming : Boolean = false,
    val typeName : String = "NONE",
    val subTypeName : String = "NONE",
    val reason : String? = null,
    val extraInfo : String? = null
) {
    private constructor(info: NetworkInfo) : this(
        info.state,
        info.detailedState,
        info.type,
        info.subtype,
        info.isAvailable,
        info.isFailover,
        info.isRoaming,
        info.typeName,
        info.subtypeName,
        info.reason,
        info.extraInfo
    )

    companion object {
        fun create(manager: ConnectivityManager): Connectivity {
            return manager.activeNetworkInfo?.let { Connectivity(it) } ?: Connectivity()
        }

        @RequiresApi(21)
        fun create(manager: ConnectivityManager, network: Network): Connectivity {
            return manager.getNetworkInfo(network)?.let { Connectivity(it) } ?: Connectivity()
        }
    }
}
