package com.topjohnwu.magisk.model.entity.internal

import android.content.Context
import android.os.Parcelable
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.extensions.cachedFile
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.module.Repo
import kotlinx.android.parcel.IgnoredOnParcel
import kotlinx.android.parcel.Parcelize
import java.io.File

sealed class DownloadSubject : Parcelable {

    abstract val url: String
    abstract val file: File
    open val title: String get() = file.name

    @Parcelize
    data class Module(
        val module: Repo,
        val configuration: Configuration
    ) : DownloadSubject() {
        override val url: String get() = module.zipUrl

        @IgnoredOnParcel
        override val file by lazy {
            File(Config.downloadDirectory, module.downloadFilename)
        }
    }

    @Parcelize
    data class Manager(
        val configuration: Configuration.APK
    ) : DownloadSubject() {

        @IgnoredOnParcel
        val manager: ManagerJson = Info.remote.app

        override val title: String
            get() = "MagiskManager-v${manager.version}(${manager.versionCode})"

        override val url: String
            get() = manager.link

        @IgnoredOnParcel
        override val file by lazy {
            get<Context>().cachedFile("manager.apk")
        }

    }

    sealed class Magisk : DownloadSubject() {

        abstract val configuration: Configuration
        val magisk: MagiskJson = Info.remote.magisk

        @Parcelize
        protected data class Flash(
            override val configuration: Configuration
        ) : Magisk() {
            override val url: String get() = magisk.link
            override val title: String get() = "Magisk-v${magisk.version}(${magisk.versionCode})"

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile("magisk.zip")
            }
        }

        @Parcelize
        protected class Uninstall : Magisk() {
            override val configuration: Configuration get() = Configuration.Uninstall
            override val url: String get() = Info.remote.uninstaller.link

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile("uninstall.zip")
            }
        }

        @Parcelize
        protected class Download : Magisk() {
            override val configuration: Configuration get() = Configuration.Download
            override val url: String get() = magisk.link

            @IgnoredOnParcel
            override val file by lazy {
                File(Config.downloadDirectory, "Magisk-v${magisk.version}(${magisk.versionCode}).zip")
            }
        }

        companion object {
            operator fun invoke(configuration: Configuration) = when (configuration) {
                Configuration.Download -> Download()
                Configuration.Uninstall -> Uninstall()
                else -> Flash(configuration)
            }
        }

    }

}