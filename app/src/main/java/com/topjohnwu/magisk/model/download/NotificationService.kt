package com.topjohnwu.magisk.model.download

import android.app.Notification
import android.content.Intent
import android.os.IBinder
import com.topjohnwu.magisk.base.BaseService
import com.topjohnwu.magisk.view.Notifications
import org.koin.core.KoinComponent
import java.util.*
import kotlin.random.Random.Default.nextInt

abstract class NotificationService : BaseService(), KoinComponent {

    abstract val defaultNotification: Notification.Builder

    private val hasNotifications get() = notifications.isNotEmpty()

    private val notifications =
        Collections.synchronizedMap(mutableMapOf<Int, Notification.Builder>())

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { cancel(it.key) }
        notifications.clear()
    }

    // --

    fun update(
        id: Int,
        body: (Notification.Builder) -> Unit = {}
    ) {
        val notification = notifications.getOrPut(id) { defaultNotification }

        notify(id, notification.also(body).build())

        if (notifications.size == 1) {
            updateForeground()
        }
    }

    protected fun finishNotify(
        id: Int,
        editBody: (Notification.Builder) -> Notification.Builder? = { null }
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
        Notifications.mgr.notify(id, notification)
    }

    private fun cancel(id: Int) {
        Notifications.mgr.cancel(id)
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
