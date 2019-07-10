package com.topjohnwu.magisk.model.download

import android.annotation.SuppressLint
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import androidx.core.app.NotificationCompat
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.ui.flash.FlashActivity
import java.io.File
import kotlin.random.Random.Default.nextInt

@SuppressLint("Registered")
open class CompoundDownloadService : SubstrateDownloadService() {

    private val context get() = this

    override fun onFinished(file: File, subject: DownloadSubject) = when (subject) {
        is DownloadSubject.Magisk -> onFinishedInternal(file, subject)
        is DownloadSubject.Module -> onFinishedInternal(file, subject)
    }

    private fun onFinishedInternal(
        file: File,
        subject: DownloadSubject.Magisk
    ) = when (subject.configuration) {
        Configuration.FLASH -> FlashActivity.flash(this, file)
        else -> Unit
    }

    private fun onFinishedInternal(
        file: File,
        subject: DownloadSubject.Module
    ) = when (subject.configuration) {
        Configuration.FLASH -> FlashActivity.install(this, file)
        else -> Unit
    }

    // ---

    override fun NotificationCompat.Builder.addActions(
        file: File,
        subject: DownloadSubject
    ) = when (subject) {
        is DownloadSubject.Magisk -> addActionsInternal(file, subject)
        is DownloadSubject.Module -> addActionsInternal(file, subject)
    }

    private fun NotificationCompat.Builder.addActionsInternal(
        file: File,
        subject: DownloadSubject.Magisk
    ) = when (subject.configuration) {
        Configuration.FLASH -> setContentIntent(FlashActivity.flashIntent(context, file))
        else -> this
    }

    private fun NotificationCompat.Builder.addActionsInternal(
        file: File,
        subject: DownloadSubject.Module
    ) = when (subject.configuration) {
        Configuration.FLASH -> setContentIntent(FlashActivity.installIntent(context, file))
        else -> this
    }

    @Suppress("ReplaceSingleLineLet")
    private fun NotificationCompat.Builder.setContentIntent(intent: Intent) =
        PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
            .let { setContentIntent(it) }

    companion object {

        fun download(context: Context, subject: DownloadSubject) =
            Intent(context, ClassMap[CompoundDownloadService::class.java])
                .putExtra(ARG_URL, subject)
                .let { context.startService(it); Unit }

    }

}