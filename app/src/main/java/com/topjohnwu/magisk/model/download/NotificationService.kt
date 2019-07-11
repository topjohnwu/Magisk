package com.topjohnwu.magisk.model.download

import android.app.Notification
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.IBinder
import androidx.core.app.NotificationCompat
import androidx.core.content.getSystemService
import java.util.*
import kotlin.random.Random.Default.nextInt

abstract class NotificationService : Service() {

    abstract val defaultNotification: NotificationCompat.Builder

    private val manager get() = getSystemService<NotificationManager>()
    private val hasNotifications get() = notifications.isEmpty()

    private val notifications =
        Collections.synchronizedMap(mutableMapOf<Int, NotificationCompat.Builder>())

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.values.forEach { cancel(it.hashCode()) }
        notifications.clear()
    }

    // --

    protected fun update(
        id: Int,
        body: (NotificationCompat.Builder) -> Unit = {}
    ) {
        val notification = notifications.getOrPut(id) { defaultNotification }

        notify(id, notification.also(body).build())

        if (notifications.size == 1) {
            updateForeground()
        }
    }

    protected fun finishWork(
        id: Int,
        editBody: (NotificationCompat.Builder) -> NotificationCompat.Builder? = { null }
    ) {
        val currentNotification = remove(id)?.run(editBody) ?: let {
            cancel(id)
            return
        }

        updateForeground()

        cancel(id)
        notify(nextInt(), currentNotification.build())

        if (!hasNotifications) {
            stopForeground(true)
            stopSelf()
        }
    }

    // ---

    private fun notify(id: Int, notification: Notification) {
        manager?.notify(id, notification)
    }

    private fun cancel(id: Int) {
        manager?.cancel(id)
    }

    private fun remove(id: Int) = notifications.remove(id)
        .also { updateForeground() }

    private fun updateForeground() {
        runCatching { notifications.keys.first() to notifications.values.first() }
            .getOrNull()
            ?.let { startForeground(it.first, it.second.build()) }
    }

    // --

    override fun onBind(p0: Intent?): IBinder? = null
}