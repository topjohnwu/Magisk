package com.topjohnwu.magisk.view

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build.VERSION.SDK_INT
import androidx.core.content.getSystemService
import androidx.core.graphics.drawable.toIcon
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.getBitmap
import java.util.concurrent.atomic.AtomicInteger

@Suppress("DEPRECATION")
object Notifications {

    val mgr by lazy { AppContext.getSystemService<NotificationManager>()!! }

    private const val APP_UPDATED_NOTIFICATION_ID = 4
    private const val APP_UPDATE_NOTIFICATION_ID = 5

    private const val UPDATE_CHANNEL = "update"
    private const val PROGRESS_CHANNEL = "progress"
    private const val UPDATED_CHANNEL = "updated"

    private val nextId = AtomicInteger(APP_UPDATE_NOTIFICATION_ID)

    fun setup(context: Context) {
        if (SDK_INT >= 26) {
            val channel = NotificationChannel(UPDATE_CHANNEL,
                context.getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT)
            val channel2 = NotificationChannel(PROGRESS_CHANNEL,
                context.getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW)
            val channel3 = NotificationChannel(UPDATED_CHANNEL,
                context.getString(R.string.updated_channel), NotificationManager.IMPORTANCE_HIGH)
            mgr.createNotificationChannels(listOf(channel, channel2, channel3))
        }
    }

    fun selfLaunchIntent(context: Context): Intent {
        val pm = context.packageManager
        val intent = pm.getLaunchIntentForPackage(context.packageName)!!
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
        return intent
    }

    @SuppressLint("InlinedApi")
    fun updateDone(context: Context) {
        setup(context)
        val flag = PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        val pending = PendingIntent.getActivity(context, 0, selfLaunchIntent(context), flag)
        val builder = if (SDK_INT >= 26) {
            Notification.Builder(context, UPDATED_CHANNEL)
                .setSmallIcon(context.getBitmap(R.drawable.ic_magisk_outline).toIcon())
        } else {
            Notification.Builder(context).setPriority(Notification.PRIORITY_HIGH)
                .setSmallIcon(R.drawable.ic_magisk_outline)
        }
            .setContentIntent(pending)
            .setContentTitle(context.getText(R.string.updated_title))
            .setContentText(context.getText(R.string.updated_text))
            .setAutoCancel(true)
        mgr.notify(APP_UPDATED_NOTIFICATION_ID, builder.build())
    }

    fun updateAvailable(context: Context) {
        val intent = DownloadService.getPendingIntent(context, Subject.App())

        val bitmap = context.getBitmap(R.drawable.ic_magisk_outline)
        val builder = if (SDK_INT >= 26) {
            Notification.Builder(context, UPDATE_CHANNEL)
                .setSmallIcon(bitmap.toIcon())
        } else {
            Notification.Builder(context)
                .setSmallIcon(R.drawable.ic_magisk_outline)
        }
            .setLargeIcon(bitmap)
            .setContentTitle(context.getString(R.string.magisk_update_title))
            .setContentText(context.getString(R.string.manager_download_install))
            .setAutoCancel(true)
            .setContentIntent(intent)

        mgr.notify(APP_UPDATE_NOTIFICATION_ID, builder.build())
    }

    fun progress(context: Context, title: CharSequence): Notification.Builder {
        val builder = if (SDK_INT >= 26) {
            Notification.Builder(context, PROGRESS_CHANNEL)
        } else {
            Notification.Builder(context).setPriority(Notification.PRIORITY_LOW)
        }
            .setSmallIcon(android.R.drawable.stat_sys_download)
            .setContentTitle(title)
            .setProgress(0, 0, true)
            .setOngoing(true)
        return builder
    }

    fun nextId() = nextId.incrementAndGet()
}
