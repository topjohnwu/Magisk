package com.topjohnwu.magisk.view

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build.VERSION.SDK_INT
import androidx.core.content.getSystemService
import androidx.core.graphics.drawable.toIcon
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const.ID.PROGRESS_NOTIFICATION_CHANNEL
import com.topjohnwu.magisk.core.Const.ID.UPDATE_NOTIFICATION_CHANNEL
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.getBitmap
import java.util.concurrent.atomic.AtomicInteger

@Suppress("DEPRECATION")
object Notifications {

    val mgr by lazy { AppContext.getSystemService<NotificationManager>()!! }

    private const val APK_UPDATE_NOTIFICATION_ID = 5
    private val nextId = AtomicInteger(APK_UPDATE_NOTIFICATION_ID)

    fun setup(context: Context) {
        if (SDK_INT >= 26) {
            val channel = NotificationChannel(UPDATE_NOTIFICATION_CHANNEL,
                context.getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT)
            val channel2 = NotificationChannel(PROGRESS_NOTIFICATION_CHANNEL,
                context.getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW)
            mgr.createNotificationChannels(listOf(channel, channel2))
        }
    }

    private fun updateBuilder(context: Context): Notification.Builder {
        return Notification.Builder(context).apply {
            val bitmap = context.getBitmap(R.drawable.ic_magisk_outline)
            setLargeIcon(bitmap)
            if (SDK_INT >= 26) {
                setSmallIcon(bitmap.toIcon())
                setChannelId(UPDATE_NOTIFICATION_CHANNEL)
            } else {
                setSmallIcon(R.drawable.ic_magisk_outline)
            }
        }
    }

    fun managerUpdate(context: Context) {
        val intent = DownloadService.getPendingIntent(context, Subject.App())

        val builder = updateBuilder(context)
            .setContentTitle(context.getString(R.string.magisk_update_title))
            .setContentText(context.getString(R.string.manager_download_install))
            .setAutoCancel(true)
            .setContentIntent(intent)

        mgr.notify(APK_UPDATE_NOTIFICATION_ID, builder.build())
    }

    fun progress(context: Context, title: CharSequence): Notification.Builder {
        val builder = if (SDK_INT >= 26) {
            Notification.Builder(context, PROGRESS_NOTIFICATION_CHANNEL)
        } else {
            Notification.Builder(context).setPriority(Notification.PRIORITY_LOW)
        }
        builder.setSmallIcon(android.R.drawable.stat_sys_download)
            .setContentTitle(title)
            .setProgress(0, 0, true)
            .setOngoing(true)
        return builder
    }

    fun nextId() = nextId.incrementAndGet()
}
