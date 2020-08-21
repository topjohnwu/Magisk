package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import com.topjohnwu.magisk.core.download.Configuration.*
import com.topjohnwu.magisk.core.download.Configuration.Flash.Secondary
import com.topjohnwu.magisk.core.download.DownloadSubject.*
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.tasks.EnvFixTask
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.APKInstall
import kotlin.random.Random.Default.nextInt

@SuppressLint("Registered")
open class DownloadService : BaseDownloadService() {

    private val context get() = this

    override suspend fun onFinish(subject: DownloadSubject, id: Int) = when (subject) {
        is Magisk -> subject.onFinish(id)
        is Module -> subject.onFinish(id)
        is Manager -> subject.onFinish(id)
    }

    private suspend fun Magisk.onFinish(id: Int) = when (val conf = configuration) {
        Uninstall -> FlashFragment.uninstall(file, id)
        EnvFix -> {
            cancel(id)
            EnvFixTask(file).exec()
            Unit
        }
        is Patch -> FlashFragment.patch(file, conf.fileUri, id)
        is Flash -> FlashFragment.flash(file, conf is Secondary, id)
        else -> Unit
    }

    private fun Module.onFinish(id: Int) = when (configuration) {
        is Flash -> FlashFragment.install(file, id)
        else -> Unit
    }

    private suspend fun Manager.onFinish(id: Int) {
        handleAPK(this)
        cancel(id)
    }

    // --- Customize finish notification

    override fun Notification.Builder.setIntent(subject: DownloadSubject)
    = when (subject) {
        is Magisk -> setIntent(subject)
        is Module -> setIntent(subject)
        is Manager -> setIntent(subject)
    }

    private fun Notification.Builder.setIntent(subject: Magisk)
    = when (val conf = subject.configuration) {
        Uninstall -> setContentIntent(FlashFragment.uninstallIntent(context, subject.file))
        is Flash -> setContentIntent(FlashFragment.flashIntent(context, subject.file, conf is Secondary))
        is Patch -> setContentIntent(FlashFragment.patchIntent(context, subject.file, conf.fileUri))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setIntent(subject: Module)
    = when (subject.configuration) {
        is Flash -> setContentIntent(FlashFragment.installIntent(context, subject.file))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setIntent(subject: Manager)
    = when (subject.configuration) {
        APK.Upgrade -> setContentIntent(APKInstall.installIntent(context, subject.file))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setContentIntent(intent: Intent) =
        setContentIntent(
            PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
        )

    // ---

    class Builder {
        lateinit var subject: DownloadSubject
    }

    companion object {

        inline operator fun invoke(context: Context, argBuilder: Builder.() -> Unit) {
            val app = context.applicationContext
            val builder = Builder().apply(argBuilder)
            val intent = app.intent<DownloadService>().putExtra(ARG_URL, builder.subject)

            if (Build.VERSION.SDK_INT >= 26) {
                app.startForegroundService(intent)
            } else {
                app.startService(intent)
            }
        }

    }

}
