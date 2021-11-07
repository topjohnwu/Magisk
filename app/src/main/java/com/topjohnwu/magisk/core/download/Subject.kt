package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Parcelable
import androidx.core.net.toFile
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.StubJson
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.cachedFile
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.magisk.view.Notifications
import kotlinx.parcelize.IgnoredOnParcel
import kotlinx.parcelize.Parcelize

private fun cachedFile(name: String) = AppContext.cachedFile(name).apply { delete() }.toUri()

enum class Action {
    Flash,
    Download
}

sealed class Subject : Parcelable {

    abstract val url: String
    abstract val file: Uri
    abstract val title: String
    abstract val notifyId: Int

    abstract fun pendingIntent(context: Context): PendingIntent

    @Parcelize
    class Module(
        val module: OnlineModule,
        val action: Action,
        override val notifyId: Int = Notifications.nextId()
    ) : Subject() {
        override val url: String get() = module.zip_url
        override val title: String get() = module.downloadFilename

        @IgnoredOnParcel
        override val file by lazy {
            MediaStoreUtils.getFile(title).uri
        }

        override fun pendingIntent(context: Context) = when (action) {
            Action.Flash -> FlashFragment.installIntent(context, file)
            else -> Intent().toPending(context)
        }
    }

    @Parcelize
    class Manager(
        private val json: MagiskJson = Info.remote.magisk,
        val stub: StubJson = Info.remote.stub,
        override val notifyId: Int = Notifications.nextId()
    ) : Subject() {
        override val title: String get() = "Magisk-${json.version}(${json.versionCode})"
        override val url: String get() = json.link

        @IgnoredOnParcel
        override val file by lazy {
            cachedFile("manager.apk")
        }

        val externalFile get() = MediaStoreUtils.getFile("$title.apk").uri

        override fun pendingIntent(context: Context) =
            APKInstall.installIntent(context, file.toFile()).toPending(context)
    }

    @SuppressLint("InlinedApi")
    protected fun Intent.toPending(context: Context): PendingIntent {
        return PendingIntent.getActivity(context, notifyId, this,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_ONE_SHOT)
    }
}
