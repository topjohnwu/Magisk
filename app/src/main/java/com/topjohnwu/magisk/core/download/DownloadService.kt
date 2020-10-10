package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.core.net.toFile
import com.topjohnwu.magisk.core.download.Action.*
import com.topjohnwu.magisk.core.download.Action.Flash.Secondary
import com.topjohnwu.magisk.core.download.Subject.*
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.tasks.EnvFixTask
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.APKInstall
import kotlin.random.Random.Default.nextInt

@SuppressLint("Registered")
open class DownloadService : BaseDownloader() {

    private val context get() = this

    override suspend fun onFinish(subject: Subject, id: Int) = when (subject) {
        is Magisk -> subject.onFinish(id)
        is Module -> subject.onFinish(id)
        is Manager -> subject.onFinish(id)
    }

    private suspend fun Magisk.onFinish(id: Int) = when (val action = action) {
        Uninstall -> FlashFragment.uninstall(file, id)
        EnvFix -> {
            remove(id)
            EnvFixTask(file).exec()
            Unit
        }
        is Patch -> FlashFragment.patch(file, action.fileUri, id)
        is Flash -> FlashFragment.flash(file, action is Secondary, id)
        else -> Unit
    }

    private fun Module.onFinish(id: Int) = when (action) {
        is Flash -> FlashFragment.install(file, id)
        else -> Unit
    }

    private fun Manager.onFinish(id: Int) {
        remove(id)
        APKInstall.install(context, file.toFile())
    }

    // --- Customize finish notification

    override fun Notification.Builder.setIntent(subject: Subject)
    = when (subject) {
        is Magisk -> setIntent(subject)
        is Module -> setIntent(subject)
        is Manager -> setIntent(subject)
    }

    private fun Notification.Builder.setIntent(subject: Magisk)
    = when (val action = subject.action) {
        Uninstall -> setContentIntent(FlashFragment.uninstallIntent(context, subject.file))
        is Flash -> setContentIntent(FlashFragment.flashIntent(context, subject.file, action is Secondary))
        is Patch -> setContentIntent(FlashFragment.patchIntent(context, subject.file, action.fileUri))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setIntent(subject: Module)
    = when (subject.action) {
        is Flash -> setContentIntent(FlashFragment.installIntent(context, subject.file))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setIntent(subject: Manager)
    = when (subject.action) {
        APK.Upgrade -> setContentIntent(APKInstall.installIntent(context, subject.file.toFile()))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setContentIntent(intent: Intent) =
        setContentIntent(
            PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
        )

    // ---

    companion object {

        private fun intent(context: Context, subject: Subject) =
            context.intent<DownloadService>().putExtra(ACTION_KEY, subject)

        fun pendingIntent(context: Context, subject: Subject): PendingIntent {
            return if (Build.VERSION.SDK_INT >= 26) {
                PendingIntent.getForegroundService(context, nextInt(),
                    intent(context, subject), PendingIntent.FLAG_UPDATE_CURRENT)
            } else {
                PendingIntent.getService(context, nextInt(),
                    intent(context, subject), PendingIntent.FLAG_UPDATE_CURRENT)
            }
        }

        fun start(context: Context, subject: Subject) {
            val app = context.applicationContext
            if (Build.VERSION.SDK_INT >= 26) {
                app.startForegroundService(intent(app, subject))
            } else {
                app.startService(intent(app, subject))
            }
        }
    }

}
