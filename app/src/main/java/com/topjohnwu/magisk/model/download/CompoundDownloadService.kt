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

    override fun onFinished(file: File, subject: DownloadSubject) {
        when (subject) {
            is DownloadSubject.Magisk -> {
                if (subject.configuration == Configuration.FLASH) {
                    FlashActivity.flashMagisk(this, file)
                }
            }
            is DownloadSubject.Module -> {
                if (subject.configuration == Configuration.FLASH) {
                    FlashActivity.flashModule(this, file)
                }
            }
        }
    }

    // ---

    override fun NotificationCompat.Builder.addActions(file: File, subject: DownloadSubject) =
        when (subject) {
            is DownloadSubject.Magisk -> addMagiskActions(file, subject.configuration)
            is DownloadSubject.Module -> addModuleActions(file, subject.configuration)
        }

    private fun NotificationCompat.Builder.addMagiskActions(
        file: File,
        configuration: Configuration
    ) = apply {
        when (configuration) {
            Configuration.FLASH -> {
                val inner = FlashActivity.flashMagiskIntent(context, file)
                val intent = PendingIntent
                    .getActivity(context, nextInt(), inner, PendingIntent.FLAG_ONE_SHOT)

                setContentIntent(intent)
            }
        }

    }

    private fun NotificationCompat.Builder.addModuleActions(
        file: File,
        configuration: Configuration
    ) = apply {

    }

    companion object {

        fun download(context: Context, subject: DownloadSubject) =
            Intent(context, ClassMap[CompoundDownloadService::class.java])
                .putExtra(ARG_URL, subject)
                .let { context.startService(it); Unit }

    }

}