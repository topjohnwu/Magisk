package com.topjohnwu.magisk.core.download

import android.net.Uri
import android.os.Parcelable
import kotlinx.parcelize.Parcelize

sealed class Action : Parcelable {

    sealed class Flash : Action() {

        @Parcelize
        object Primary : Flash()

        @Parcelize
        object Secondary : Flash()

    }

    @Parcelize
    object Download : Action()

    @Parcelize
    object Uninstall : Action()

    @Parcelize
    object EnvFix : Action()

    @Parcelize
    data class Patch(val fileUri: Uri) : Action()

}
