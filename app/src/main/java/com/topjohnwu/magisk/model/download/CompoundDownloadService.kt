package com.topjohnwu.magisk.model.download

import android.Manifest
import android.annotation.SuppressLint
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import androidx.annotation.RequiresPermission
import androidx.core.app.NotificationCompat
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.model.entity.internal.Configuration.*
import com.topjohnwu.magisk.model.entity.internal.Configuration.Flash.Secondary
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Magisk
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Module
import com.topjohnwu.magisk.ui.flash.FlashActivity
import java.io.File
import kotlin.random.Random.Default.nextInt

@SuppressLint("Registered")
open class CompoundDownloadService : SubstrateDownloadService() {

    private val context get() = this

    override fun onFinished(file: File, subject: DownloadSubject) = when (subject) {
        is Magisk -> onFinishedInternal(file, subject)
        is Module -> onFinishedInternal(file, subject)
    }

    private fun onFinishedInternal(
        file: File,
        subject: Magisk
    ) = when (val conf = subject.configuration) {
        Download -> moveToDownloads(file)
        Uninstall -> FlashActivity.uninstall(this, file)
        is Patch -> FlashActivity.patch(this, file, conf.fileUri)
        is Flash -> FlashActivity.flash(this, file, conf is Secondary)
        else -> Unit
    }

    private fun onFinishedInternal(
        file: File,
        subject: Module
    ) = when (subject.configuration) {
        Download -> moveToDownloads(file)
        is Flash -> FlashActivity.install(this, file)
        else -> Unit
    }

    // ---

    override fun NotificationCompat.Builder.addActions(
        file: File,
        subject: DownloadSubject
    ) = when (subject) {
        is Magisk -> addActionsInternal(file, subject)
        is Module -> addActionsInternal(file, subject)
    }

    private fun NotificationCompat.Builder.addActionsInternal(
        file: File,
        subject: Magisk
    ) = when (val conf = subject.configuration) {
        Download -> setContentIntent(fileIntent(subject.fileName))
        Uninstall -> setContentIntent(FlashActivity.uninstallIntent(context, file))
        is Flash -> setContentIntent(FlashActivity.flashIntent(context, file, conf is Secondary))
        is Patch -> setContentIntent(FlashActivity.patchIntent(context, file, conf.fileUri))
        else -> this
    }

    private fun NotificationCompat.Builder.addActionsInternal(
        file: File,
        subject: Module
    ) = when (subject.configuration) {
        Download -> setContentIntent(fileIntent(subject.fileName))
        is Flash -> setContentIntent(FlashActivity.installIntent(context, file))
        else -> this
    }

    @Suppress("ReplaceSingleLineLet")
    private fun NotificationCompat.Builder.setContentIntent(intent: Intent) =
        PendingIntent.getActivity(context, nextInt(), intent, PendingIntent.FLAG_ONE_SHOT)
            .let { setContentIntent(it) }

    companion object {

        @RequiresPermission(allOf = [Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE])
        fun download(context: Context, subject: DownloadSubject) =
            Intent(context, ClassMap[CompoundDownloadService::class.java])
                .putExtra(ARG_URL, subject)
                .let { context.startService(it); Unit }

    }

}