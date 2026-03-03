package com.topjohnwu.magisk.ui.navigation

import android.net.Uri
import android.os.Parcelable
import androidx.navigation3.runtime.NavKey
import kotlinx.parcelize.Parcelize
import kotlinx.serialization.Contextual
import kotlinx.serialization.Serializable

sealed interface Route : NavKey, Parcelable {
    @Parcelize
    @Serializable
    data object Main : Route

    @Parcelize
    @Serializable
    data object Install : Route

    @Parcelize
    @Serializable
    data object DenyList : Route

    @Parcelize
    @Serializable
    data class Flash(
        val action: String,
        val additionalData: String? = null,
    ) : Route

    @Parcelize
    @Serializable
    data class Action(
        val id: String,
        val name: String,
    ) : Route
}
