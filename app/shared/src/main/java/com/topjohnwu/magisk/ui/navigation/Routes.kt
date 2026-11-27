package com.topjohnwu.magisk.ui.navigation

import android.os.Parcelable
import androidx.navigation3.runtime.NavKey
import kotlinx.parcelize.Parcelize

sealed interface Route : NavKey, Parcelable {
    @Parcelize
    data object Main : Route

    @Parcelize
    data object DenyList : Route

    @Parcelize
    data class Flash(
        val action: String,
        val additionalData: String? = null,
        val usbDevice: String? = null,
    ) : Route

    @Parcelize
    data class SuperuserDetail(val uid: Int) : Route

    @Parcelize
    data object SuperuserGrant : Route

    @Parcelize
    data class Action(
        // The module's directory name: unlike the id in module.prop, it is unique
        val dir: String,
        val name: String,
    ) : Route
}
