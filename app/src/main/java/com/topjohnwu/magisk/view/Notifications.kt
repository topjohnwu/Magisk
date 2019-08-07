package com.topjohnwu.magisk.view

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.app.TaskStackBuilder
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.ui.SplashActivity

object Notifications {

    val mgr by lazy { NotificationManagerCompat.from(get()) }

    fun setup(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            mgr.deleteNotificationChannel("magisk_notification")
            var channel = NotificationChannel(Const.ID.UPDATE_NOTIFICATION_CHANNEL,
                    context.getString(R.string.update_channel), NotificationManager.IMPORTANCE_DEFAULT)
            mgr.createNotificationChannel(channel)
            channel = NotificationChannel(Const.ID.PROGRESS_NOTIFICATION_CHANNEL,
                    context.getString(R.string.progress_channel), NotificationManager.IMPORTANCE_LOW)
            mgr.createNotificationChannel(channel)
        }
    }

    fun magiskUpdate(context: Context) {
        val intent = Intent(context, ClassMap[SplashActivity::class.java])
        intent.putExtra(Const.Key.OPEN_SECTION, "magisk")
        val stackBuilder = TaskStackBuilder.create(context)
        stackBuilder.addParentStack(ClassMap.get<Class<*>>(SplashActivity::class.java))
        stackBuilder.addNextIntent(intent)
        val pendingIntent = stackBuilder.getPendingIntent(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID,
                PendingIntent.FLAG_UPDATE_CURRENT)

        val builder = NotificationCompat.Builder(context, Const.ID.UPDATE_NOTIFICATION_CHANNEL)
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(context.getString(R.string.magisk_update_title))
                .setContentText(context.getString(R.string.manager_download_install))
                .setVibrate(longArrayOf(0, 100, 100, 100))
                .setAutoCancel(true)
                .setContentIntent(pendingIntent)

        mgr.notify(Const.ID.MAGISK_UPDATE_NOTIFICATION_ID, builder.build())
    }

    fun managerUpdate(context: Context) {
        val intent = Intent(context, ClassMap[GeneralReceiver::class.java])
        intent.action = Const.Key.BROADCAST_MANAGER_UPDATE
        intent.putExtra(Const.Key.INTENT_SET_APP, Info.remote.app)

        val pendingIntent = PendingIntent.getBroadcast(context,
                Const.ID.APK_UPDATE_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT)

        val builder = NotificationCompat.Builder(context, Const.ID.UPDATE_NOTIFICATION_CHANNEL)
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(context.getString(R.string.manager_update_title))
                .setContentText(context.getString(R.string.manager_download_install))
                .setVibrate(longArrayOf(0, 100, 100, 100))
                .setAutoCancel(true)
                .setContentIntent(pendingIntent)

        mgr.notify(Const.ID.APK_UPDATE_NOTIFICATION_ID, builder.build())
    }

    fun dtboPatched(context: Context) {
        val intent = Intent(context, ClassMap[GeneralReceiver::class.java])
                .setAction(Const.Key.BROADCAST_REBOOT)
        val pendingIntent = PendingIntent.getBroadcast(context,
                Const.ID.DTBO_NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT)

        val builder = NotificationCompat.Builder(context, Const.ID.UPDATE_NOTIFICATION_CHANNEL)
        builder.setSmallIcon(R.drawable.ic_magisk_outline)
                .setContentTitle(context.getString(R.string.dtbo_patched_title))
                .setContentText(context.getString(R.string.dtbo_patched_reboot))
                .setVibrate(longArrayOf(0, 100, 100, 100))
                .addAction(R.drawable.ic_refresh, context.getString(R.string.reboot), pendingIntent)

        mgr.notify(Const.ID.DTBO_NOTIFICATION_ID, builder.build())
    }

    fun progress(context: Context, title: CharSequence): NotificationCompat.Builder {
        val builder = NotificationCompat.Builder(context, Const.ID.PROGRESS_NOTIFICATION_CHANNEL)
        builder.setPriority(NotificationCompat.PRIORITY_LOW)
                .setSmallIcon(android.R.drawable.stat_sys_download)
                .setContentTitle(title)
                .setProgress(0, 0, true)
                .setOngoing(true)
        return builder
    }
}
