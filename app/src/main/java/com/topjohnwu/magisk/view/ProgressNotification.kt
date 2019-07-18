package com.topjohnwu.magisk.view

import android.app.Notification
import android.app.PendingIntent
import android.content.Intent
import android.widget.Toast

import androidx.core.app.NotificationCompat

import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.net.DownloadProgressListener

class ProgressNotification(title: String) : DownloadProgressListener {

    val notificationBuilder: NotificationCompat.Builder = Notifications.progress(title)
    lateinit var notification: Notification
        private set
    private var prevTime: Long = 0

    init {
        prevTime = System.currentTimeMillis()
        update()
        Utils.toast(App.self.getString(R.string.downloading_toast, title), Toast.LENGTH_SHORT)
    }

    override fun onProgress(bytesDownloaded: Long, totalBytes: Long) {
        val cur = System.currentTimeMillis()
        if (cur - prevTime >= 1000) {
            prevTime = cur
            val progress = (bytesDownloaded * 100 / totalBytes).toInt()
            notificationBuilder.setProgress(100, progress, false)
            notificationBuilder.setContentText("$progress%")
            update()
        }
    }

    fun update() {
        notification = notificationBuilder.build()
        Notifications.mgr.notify(hashCode(), notification)
    }

    private fun lastUpdate() {
        Notifications.mgr.cancel(hashCode())
        notification = notificationBuilder.build().apply {
            Notifications.mgr.notify(hashCode(), this)
        }
    }

    fun dlDone(intent: PendingIntent = PendingIntent.getActivity(App.self, hashCode(),
            Intent(), PendingIntent.FLAG_UPDATE_CURRENT)) {
        notificationBuilder.setProgress(0, 0, false)
                .setContentText(App.self.getString(R.string.download_complete))
                .setSmallIcon(android.R.drawable.stat_sys_download_done)
                .setContentIntent(intent)
                .setOngoing(false)
                .setAutoCancel(true)
        lastUpdate()
    }

    fun dlFail() {
        notificationBuilder.setProgress(0, 0, false)
                .setContentText(App.self.getString(R.string.download_file_error))
                .setSmallIcon(android.R.drawable.stat_notify_error)
                .setOngoing(false)
        lastUpdate()
    }

    fun dismiss() {
        Notifications.mgr.cancel(hashCode())
    }
}
