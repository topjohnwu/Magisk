package com.topjohnwu.magisk.model.entity.internal

import android.os.Parcelable
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.Repo
import kotlinx.android.parcel.Parcelize

sealed class DownloadSubject : Parcelable {

    abstract val fileName: String
    abstract val url: String

    @Parcelize
    data class Module(
        val module: Repo,
        val configuration: Configuration
    ) : DownloadSubject() {

        override val url: String get() = module.zipUrl
        override val fileName: String get() = "${module.name}-v${module.version}(${module.versionCode}).zip"

    }

    @Parcelize
    data class Magisk(
        val configuration: Configuration,
        val magisk: MagiskJson = Info.remote.magisk
    ) : DownloadSubject() {

        override val url: String get() = magisk.link
        override val fileName get() = "Magisk-v${magisk.version}(${magisk.versionCode}).zip"

    }

    @Parcelize
    object Installer : DownloadSubject() {
        override val fileName: String get() = "module_installer(${BuildConfig.VERSION_CODE}).sh"
        override val url: String get() = Const.Url.MODULE_INSTALLER
    }

}