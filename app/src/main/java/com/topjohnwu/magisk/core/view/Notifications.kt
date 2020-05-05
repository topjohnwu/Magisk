package com.topjohnwu.magisk.core.view

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.os.Build.VERSION.SDK_INT
import androidx.core.app.TaskStackBuilder
import androidx.core.content.getSystemService
import androidx.core.graphics.drawable.toIcon
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.*
import com.topjohnwu.magisk.core.Const.ID.PROGRESS_NOTIFICATION_CHANNEL
import com.topjohnwu.magisk.core.Const.ID.UPDATE_NOTIFICATION_CHANNEL
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.getBitmap

object Notifications {

    val mgr by lazy { get<Context>().getSystemService<NotificationManager>()!! }

    fun setup(context: Context) {
        if (SDK_INT >= 26) {
            var channel = NotificationChannel(UPDATE_NOTIFICATION_CHANNEL,
                    context.getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT)
            mgr.createNotificationChannel(channel)
            channel = NotificationChannel(PROGRESS_NOTIFICATION_CHANNEL,
                    context.getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW)
            mgr.createNotificationChannel(channel)
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
                setVibrate(longArrayOf(0, 100, 100, 100))
            }
        }
    }

    fun magiskUpdate(context: Context) {
        val intent = context.intent<SplashActivity>()
                .putExtra(Const.Key.OPEN_SECTION, "magisk")
        val stackBuilder = TaskStackBuilder.create(context)
        stackBuilder.addParentStack(SplashActivity::class.java.cmp(context.packageName))
        stackBuilder.addNextIntent(intent)
        val pendingIntent = stackBuilder.getPendingIntent(
            Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT)

        val builder = updateBuilder(
            context
        )
            .setContentTitle(context.getString(R.string.magisk_update_title))
            .setContentText(context.getString(R.string.manager_download_install))
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)

        mgr.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build())
    }

    fun managerUpdate(context: Context) {
        val intent = context.intent<GeneralReceiver>()
                .setAction(Const.Key.BROADCAST_MANAGER_UPDATE)
                .putExtra(Const.Key.INTENT_SET_APP, Info.remote.app)

        val pendingIntent = PendingIntent.getBroadcast(context,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT)

        val builder = updateBuilder(
            context
        )
            .setContentTitle(context.getString(R.string.manager_update_title))
            .setContentText(context.getString(R.string.manager_download_install))
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)

        mgr.notify(Const.ID.APK_UPDATE_NOTIFICATION_ID, builder.build())
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
}
