package com.topjohnwu.magisk.model.entity.internal

import android.os.Parcelable
import com.topjohnwu.magisk.model.entity.MagiskJson
import kotlinx.android.parcel.Parcelize
import com.topjohnwu.magisk.model.entity.Module as MagiskModule

sealed class DownloadSubject : Parcelable {

    abstract val fileName: String
    abstract val url: String

    @Parcelize
    data class Module(
        val module: MagiskModule,
        val configuration: Configuration
    ) : DownloadSubject() {

        override val url: String get() = module.path
        override val fileName: String get() = "${module.name}-v${module.version}(${module.versionCode}).zip"

    }

    @Parcelize
    data class Magisk(
        val magisk: MagiskJson,
        val configuration: Configuration
    ) : DownloadSubject() {

        override val url: String get() = magisk.link
        override val fileName get() = "Magisk-v${magisk.version}(${magisk.versionCode}).zip"

    }

}