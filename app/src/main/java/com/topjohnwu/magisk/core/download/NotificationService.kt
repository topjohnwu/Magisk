package com.topjohnwu.magisk.core.download

import android.app.Notification
import android.content.Intent
import android.os.IBinder
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.synchronized
import com.topjohnwu.magisk.view.Notifications
import okhttp3.ResponseBody
import java.io.InputStream

open class NotificationService : BaseService() {

    private val notifications = HashMap<Int, Notification.Builder>().synchronized()
    protected val hasNotifications get() = notifications.isNotEmpty()

    protected val service get() = ServiceLocator.networkService

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { Notifications.mgr.cancel(it.key) }
        notifications.clear()
    }

    protected fun ResponseBody.toProgressStream(subject: Subject): InputStream {
        val max = contentLength()
        val total = max.toFloat() / 1048576
        val id = subject.notifyId

        update(id) { it.setContentTitle(subject.title) }

        return ProgressInputStream(byteStream()) {
            val progress = it.toFloat() / 1048576
            update(id) { notification ->
                if (max > 0) {
                    broadcast(progress / total, subject)
                    notification
                        .setProgress(max.toInt(), it.toInt(), false)
                        .setContentText("%.2f / %.2f MB".format(progress, total))
                } else {
                    broadcast(-1f, subject)
                    notification.setContentText("%.2f MB / ??".format(progress))
                }
            }
        }
    }

    private fun finalNotify(id: Int, editor: (Notification.Builder) -> Unit): Int {
        val notification = remove(id)?.also(editor) ?: return -1
        val newId = Notifications.nextId()
        Notifications.mgr.notify(newId, notification.build())
        return newId
    }

    protected fun notifyFail(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(-2f, subject)
        it.setContentText(getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    protected fun notifyFinish(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(1f, subject)
        it.setContentTitle(subject.title)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
        subject.pendingIntent(this)?.let { intent -> it.setContentIntent(intent) }
    }

    private fun create() = Notifications.progress(this, "")

    private fun updateForeground() {
        if (hasNotifications) {
            val (id, notification) = notifications.entries.first()
            startForeground(id, notification.build())
        } else {
            stopForeground(false)
        }
    }

    protected fun update(id: Int, editor: (Notification.Builder) -> Unit = {}) {
        val wasEmpty = !hasNotifications
        val notification = notifications.getOrPut(id, ::create).also(editor)
        if (wasEmpty)
            updateForeground()
        else
            Notifications.mgr.notify(id, notification.build())
    }

    protected fun remove(id: Int): Notification.Builder? {
        val n = notifications.remove(id)?.also { updateForeground() }
        Notifications.mgr.cancel(id)
        return n
    }

    companion object {
        @JvmStatic
        protected val progressBroadcast = MutableLiveData<Pair<Float, Subject>?>()

        private fun broadcast(progress: Float, subject: Subject) {
            progressBroadcast.postValue(progress to subject)
        }
    }
}
