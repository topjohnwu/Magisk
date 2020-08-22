package com.topjohnwu.magisk.core.download

import android.content.Context
import android.net.Uri
import android.os.Parcelable
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.ManagerJson
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.ktx.cachedFile
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import kotlinx.android.parcel.IgnoredOnParcel
import kotlinx.android.parcel.Parcelize

sealed class Subject : Parcelable {

    abstract val url: String
    abstract val file: Uri
    abstract val action: Action
    abstract val title: String

    @Parcelize
    class Module(
        val module: Repo,
        override val action: Action
    ) : Subject() {
        override val url: String get() = module.zipUrl
        override val title: String get() = module.downloadFilename

        @IgnoredOnParcel
        override val file by lazy {
            MediaStoreUtils.getFile(title).uri
        }
    }

    @Parcelize
    class Manager(
        override val action: Action.APK
    ) : Subject() {

        @IgnoredOnParcel
        val manager: ManagerJson = Info.remote.app

        override val title: String
            get() = "MagiskManager-${manager.version}(${manager.versionCode})"

        override val url: String
            get() = manager.link

        @IgnoredOnParcel
        override val file by lazy {
            get<Context>().cachedFile("manager.apk").toUri()
        }

    }

    abstract class Magisk : Subject() {

        val magisk: MagiskJson = Info.remote.magisk

        @Parcelize
        private class Internal(
            override val action: Action
        ) : Magisk() {
            override val url: String get() = magisk.link
            override val title: String get() = "Magisk-${magisk.version}(${magisk.versionCode})"

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile("magisk.zip").toUri()
            }
        }

        @Parcelize
        private class Uninstall : Magisk() {
            override val action get() = Action.Uninstall
            override val url: String get() = Info.remote.uninstaller.link
            override val title: String get() = "uninstall.zip"

            @IgnoredOnParcel
            override val file by lazy {
                get<Context>().cachedFile(title).toUri()
            }

        }

        @Parcelize
        private class Download : Magisk() {
            override val action get() = Action.Download
            override val url: String get() = magisk.link
            override val title: String get() = "Magisk-${magisk.version}(${magisk.versionCode}).zip"

            @IgnoredOnParcel
            override val file by lazy {
                MediaStoreUtils.getFile(title).uri
            }
        }

        companion object {
            operator fun invoke(config: Action) = when (config) {
                Action.Download -> Download()
                Action.Uninstall -> Uninstall()
                Action.EnvFix, is Action.Flash, is Action.Patch -> Internal(config)
                else -> throw IllegalArgumentException()
            }
        }

    }

}
