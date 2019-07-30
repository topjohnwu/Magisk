package com.topjohnwu.magisk.model.download

import android.app.Notification
import android.app.Service
import android.content.Intent
import android.os.IBinder
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import java.util.*
import kotlin.random.Random.Default.nextInt

abstract class NotificationService : Service() {

    abstract val defaultNotification: NotificationCompat.Builder

    private val manager by lazy { NotificationManagerCompat.from(this) }
    private val hasNotifications get() = notifications.isNotEmpty()

    private val notifications =
        Collections.synchronizedMap(mutableMapOf<Int, NotificationCompat.Builder>())

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { cancel(it.key) }
        notifications.clear()
    }

    // --

    fun update(
        id: Int,
        body: (NotificationCompat.Builder) -> Unit = {}
    ) {
        val notification = notifications.getOrPut(id) { defaultNotification }

        notify(id, notification.also(body).build())

        if (notifications.size == 1) {
            updateForeground()
        }
    }

    protected fun finishNotify(
        id: Int,
        editBody: (NotificationCompat.Builder) -> NotificationCompat.Builder? = { null }
    ) : Int {
        val currentNotification = remove(id)?.run(editBody)

        var newId = -1
        currentNotification?.let {
            newId = nextInt(Int.MAX_VALUE)
            notify(newId, it.build())
        }

        if (!hasNotifications) {
            stopSelf()
        }
        return newId
    }

    // ---

    private fun notify(id: Int, notification: Notification) {
        manager.notify(id, notification)
    }

    private fun cancel(id: Int) {
        manager.cancel(id)
    }

    protected fun remove(id: Int) = notifications.remove(id).also {
        cancel(id)
        updateForeground()
    }

    private fun updateForeground() {
        if (hasNotifications)
            startForeground(notifications.keys.first(), notifications.values.first().build())
        else
            stopForeground(true)
    }

    // --

    override fun onBind(p0: Intent?): IBinder? = null
}