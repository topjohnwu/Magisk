package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.webkit.MimeTypeMap
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.tasks.EnvFixTask
import com.topjohnwu.magisk.extensions.chooser
import com.topjohnwu.magisk.extensions.exists
import com.topjohnwu.magisk.extensions.provide
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.internal.Configuration.*
import com.topjohnwu.magisk.model.entity.internal.Configuration.Flash.Secondary
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.*
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.APKInstall
import io.reactivex.Completable
import org.koin.core.get
import java.io.File
import kotlin.random.Random.Default.nextInt

/* More of a facade for [RemoteFileService], but whatever... */
@SuppressLint("Registered")
open class DownloadService : RemoteFileService() {

    private val context get() = this
    private val File.type
        get() = MimeTypeMap.getSingleton()
            .getMimeTypeFromExtension(extension)
            ?: "resource/folder"

    override fun onFinished(subject: DownloadSubject, id: Int) = when (subject) {
        is Magisk -> onFinishedInternal(subject, id)
        is Module -> onFinishedInternal(subject, id)
        is Manager -> onFinishedInternal(subject, id)
    }

    private fun onFinishedInternal(
        subject: Magisk,
        id: Int
    ) = when (val conf = subject.configuration) {
        Uninstall -> FlashFragment.uninstall(subject.file, id)
        EnvFix -> {
            remove(id); EnvFixTask(subject.file).exec()
        }
        is Patch -> FlashFragment.patch(subject.file, conf.fileUri, id)
        is Flash -> FlashFragment.flash(subject.file, conf is Secondary, id)
        else -> Unit
    }

    private fun onFinishedInternal(
        subject: Module,
        id: Int
    ) = when (subject.configuration) {
        is Flash -> FlashFragment.install(subject.file, id)
        else -> Unit
    }

    private fun onFinishedInternal(
        subject: Manager,
        id: Int
    ) {
        Completable.fromAction {
            handleAPK(subject)
        }.subscribeK {
            remove(id)
            when (subject.configuration)  {
                is APK.Upgrade -> APKInstall.install(this, subject.file)
                is APK.Restore -> Unit
            }
        }
    }

    // ---

    override fun Notification.Builder.addActions(subject: DownloadSubject)
    = when (subject) {
        is Magisk -> addActionsInternal(subject)
        is Module -> addActionsInternal(subject)
        is Manager -> addActionsInternal(subject)
    }

    private fun Notification.Builder.addActionsInternal(subject: Magisk)
    = when (val conf = subject.configuration) {
        Download -> this.apply {
            fileIntent(subject.file.parentFile!!)
                .takeIf { it.exists(get()) }
                ?.let { addAction(0, R.string.download_open_parent, it.chooser()) }
            fileIntent(subject.file)
                .takeIf { it.exists(get()) }
                ?.let { addAction(0, R.string.download_open_self, it.chooser()) }
        }
        Uninstall -> setContentIntent(FlashFragment.uninstallIntent(context, subject.file))
        is Flash -> setContentIntent(
            FlashFragment.flashIntent(
                context,
                subject.file,
                conf is Secondary
            )
        )
        is Patch -> setContentIntent(FlashFragment.patchIntent(context, subject.file, conf.fileUri))
        else -> this
    }

    private fun Notification.Builder.addActionsInternal(subject: Module)
    = when (subject.configuration) {
        Download -> this.apply {
            fileIntent(subject.file.parentFile!!)
                .takeIf { it.exists(get()) }
                ?.let { addAction(0, R.string.download_open_parent, it.chooser()) }
            fileIntent(subject.file)
                .takeIf { it.exists(get()) }
                ?.let { addAction(0, R.string.download_open_self, it.chooser()) }
        }
        is Flash -> setContentIntent(FlashFragment.installIntent(context, subject.file))
        else -> this
    }

    private fun Notification.Builder.addActionsInternal(subject: Manager)
    = when (subject.configuration) {
        APK.Upgrade -> setContentIntent(APKInstall.installIntent(context, subject.file))
        else -> this
    }

    @Suppress("ReplaceSingleLineLet")
    private fun Notification.Builder.setContentIntent(intent: Intent) =
        setContentIntent(
            PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
        )

    @Suppress("ReplaceSingleLineLet")
    private fun Notification.Builder.addAction(icon: Int, title: Int, intent: Intent) =
        addAction(icon, getString(title),
            PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
        )

    // ---

    private fun fileIntent(file: File): Intent {
        return Intent(Intent.ACTION_VIEW)
            .setDataAndType(file.provide(this), file.type)
            .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
    }

    class Builder {
        lateinit var subject: DownloadSubject
    }

    companion object {

        inline operator fun invoke(context: Context, argBuilder: Builder.() -> Unit) {
            val app = context.applicationContext
            val builder = Builder().apply(argBuilder)
            val intent = app.intent<DownloadService>().putExtra(ARG_URL, builder.subject)

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                app.startForegroundService(intent)
            } else {
                app.startService(intent)
            }
        }

    }

}
