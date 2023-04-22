package com.topjohnwu.magisk.view

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.os.Build
import android.os.Build.VERSION.SDK_INT
import androidx.core.content.getSystemService
import androidx.core.graphics.drawable.toIcon
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.di.AppContext
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.ktx.getBitmap
import com.topjohnwu.magisk.core.ktx.selfLaunchIntent
import java.util.concurrent.atomic.AtomicInteger

@Suppress("DEPRECATION")
object Notifications {

    val mgr by lazy { AppContext.getSystemService<NotificationManager>()!! }

    private const val APP_UPDATED_ID = 4
    private const val APP_UPDATE_AVAILABLE_ID = 5

    private const val UPDATE_CHANNEL = "update"
    private const val PROGRESS_CHANNEL = "progress"
    private const val UPDATED_CHANNEL = "updated"

    private val nextId = AtomicInteger(APP_UPDATE_AVAILABLE_ID)

    fun setup() {
        AppContext.apply {
            if (SDK_INT >= Build.VERSION_CODES.O) {
                val channel = NotificationChannel(UPDATE_CHANNEL,
                    getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT)
                val channel2 = NotificationChannel(PROGRESS_CHANNEL,
                    getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW)
                val channel3 = NotificationChannel(UPDATED_CHANNEL,
                    getString(R.string.updated_channel), NotificationManager.IMPORTANCE_HIGH)
                mgr.createNotificationChannels(listOf(channel, channel2, channel3))
            }
        }
    }

    @SuppressLint("InlinedApi")
    fun updateDone() {
        AppContext.apply {
            val flag = PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
            val pending = PendingIntent.getActivity(this, 0, selfLaunchIntent(), flag)
            val builder = if (SDK_INT >= Build.VERSION_CODES.O) {
                Notification.Builder(this, UPDATED_CHANNEL)
                    .setSmallIcon(getBitmap(R.drawable.ic_magisk_outline).toIcon())
            } else {
                Notification.Builder(this).setPriority(Notification.PRIORITY_HIGH)
                    .setSmallIcon(R.drawable.ic_magisk_outline)
            }
                .setContentIntent(pending)
                .setContentTitle(getText(R.string.updated_title))
                .setContentText(getText(R.string.updated_text))
                .setAutoCancel(true)
            mgr.notify(APP_UPDATED_ID, builder.build())
        }
    }

    fun updateAvailable() {
        AppContext.apply {
            val intent = DownloadService.getPendingIntent(this, Subject.App())
            val bitmap = getBitmap(R.drawable.ic_magisk_outline)
            val builder = if (SDK_INT >= Build.VERSION_CODES.O) {
                Notification.Builder(this, UPDATE_CHANNEL)
                    .setSmallIcon(bitmap.toIcon())
            } else {
                Notification.Builder(this)
                    .setSmallIcon(R.drawable.ic_magisk_outline)
            }
                .setLargeIcon(bitmap)
                .setContentTitle(getString(R.string.magisk_update_title))
                .setContentText(getString(R.string.manager_download_install))
                .setAutoCancel(true)
                .setContentIntent(intent)

            mgr.notify(APP_UPDATE_AVAILABLE_ID, builder.build())
        }
    }

    fun startProgress(title: CharSequence): Notification.Builder {
        val builder = if (SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(AppContext, PROGRESS_CHANNEL)
        } else {
            Notification.Builder(AppContext).setPriority(Notification.PRIORITY_LOW)
        }
            .setSmallIcon(android.R.drawable.stat_sys_download)
            .setContentTitle(title)
            .setProgress(0, 0, true)
            .setOngoing(true)
        if (SDK_INT >= Build.VERSION_CODES.S)
            builder.setForegroundServiceBehavior(Notification.FOREGROUND_SERVICE_IMMEDIATE)
        return builder
    }

    fun nextId() = nextId.incrementAndGet()
}
