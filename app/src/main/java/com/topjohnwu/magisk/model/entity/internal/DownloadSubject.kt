package com.topjohnwu.magisk.model.entity.internal

import android.content.Context
import android.os.Parcelable
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.extensions.cachedFile
import com.topjohnwu.magisk.extensions.get
import kotlinx.android.parcel.IgnoredOnParcel
import kotlinx.android.parcel.Parcelize
import java.io.File

sealed class DownloadSubject : Parcelable {

    abstract val url: String
    abstract val file: File
    open val title: String get() = file.name

    @Parcelize
    class Module(
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
    class Manager(
        val configuration: Configuration.APK
    ) : DownloadSubject() {

        @IgnoredOnParcel
        val manager: ManagerJson = Info.remote.app

        override val title: String
            get() = "MagiskManager-${manager.version}(${manager.versionCode})"

        override val url: String
            get() = manager.link

        @IgnoredOnParcel
        override val file by lazy {
            get<Context>().cachedFile("manager.apk")
        }

    }

    abstract class Magisk : DownloadSubject() {

        abstract val configuration: Configuration
        val magisk: MagiskJson = Info.remote.magisk

        @Parcelize
        private class DownloadInternal(
            override val configuration: Configuration
        ) : Magisk() {
            override val url: String get() = magisk.link
            override val title: String get() = "Magisk-${magisk.version}(${magisk.versionCode})"

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile("magisk.zip")
            }
        }

        @Parcelize
        private class Uninstall : Magisk() {
            override val configuration get() = Configuration.Uninstall
            override val url: String get() = Info.remote.uninstaller.link

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile("uninstall.zip")
            }
        }

        @Parcelize
        private class Download : Magisk() {
            override val configuration get() = Configuration.Download
            override val url: String get() = magisk.link

            @IgnoredOnParcel
            override val file by lazy {
                File(Config.downloadDirectory, "Magisk-${magisk.version}(${magisk.versionCode}).zip")
            }
        }

        companion object {
            operator fun invoke(configuration: Configuration) = when (configuration) {
                Configuration.Download -> Download()
                Configuration.Uninstall -> Uninstall()
                Configuration.EnvFix, is Configuration.Flash, is Configuration.Patch -> DownloadInternal(configuration)
                else -> throw IllegalArgumentException()
            }
        }

    }

}
