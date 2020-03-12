package com.topjohnwu.magisk.core.download

import android.app.Notification
import android.content.Intent
import android.os.IBinder
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.view.Notifications
import org.koin.core.KoinComponent
import java.util.*
import kotlin.collections.HashMap
import kotlin.random.Random.Default.nextInt

abstract class NotificationService : BaseService(), KoinComponent {

    private val hasNotifications get() = notifications.isNotEmpty()

    private val notifications = Collections.synchronizedMap(HashMap<Int, Notification.Builder>())

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { cancel(it.key) }
        notifications.clear()
    }

    abstract fun createNotification(): Notification.Builder

    // --

    fun update(
        id: Int,
        body: (Notification.Builder) -> Unit = {}
    ) {
        val wasEmpty = notifications.isEmpty()
        val notification = notifications.getOrPut(id, ::createNotification).also(body)
        if (wasEmpty)
            updateForeground()
        else
            notify(id, notification.build())
    }

    protected fun lastNotify(
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

    protected fun remove(id: Int) = notifications.remove(id).also {
        cancel(id)
        updateForeground()
    }

    // ---

    private fun notify(id: Int, notification: Notification) {
        Notifications.mgr.notify(id, notification)
    }

    private fun cancel(id: Int) {
        Notifications.mgr.cancel(id)
    }

    private fun updateForeground() {
        if (hasNotifications) {
            val first = notifications.entries.first()
            startForeground(first.key, first.value.build())
        } else {
            stopForeground(true)
        }
    }

    // --

    override fun onBind(p0: Intent?): IBinder? = null
}
