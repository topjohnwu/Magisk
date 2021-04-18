package com.topjohnwu.magisk.core.download

import android.app.Notification
import android.content.Intent
import android.os.IBinder
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ForegroundTracker
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.view.Notifications
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import okhttp3.ResponseBody
import timber.log.Timber
import java.io.IOException
import java.io.InputStream
import java.util.*
import kotlin.collections.HashMap
import kotlin.random.Random.Default.nextInt

abstract class BaseDownloader : BaseService() {

    private val hasNotifications get() = notifications.isNotEmpty()
    private val notifications = Collections.synchronizedMap(HashMap<Int, Notification.Builder>())
    private val coroutineScope = CoroutineScope(Dispatchers.IO)

    val service get() = ServiceLocator.networkService

    // -- Service overrides

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        intent.getParcelableExtra<Subject>(ACTION_KEY)?.let { subject ->
            update(subject.notifyID())
            coroutineScope.launch {
                try {
                    subject.startDownload()
                } catch (e: IOException) {
                    Timber.e(e)
                    notifyFail(subject)
                }
            }
        }
        return START_REDELIVER_INTENT
    }

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { cancel(it.key) }
        notifications.clear()
    }

    override fun onDestroy() {
        super.onDestroy()
        coroutineScope.cancel()
    }

    // -- Download logic

    private suspend fun Subject.startDownload() {
        val stream = service.fetchFile(url).toProgressStream(this)
        when (this) {
            is Subject.Module ->  // Download and process on-the-fly
                stream.toModule(file, service.fetchInstaller().byteStream())
            is Subject.Manager -> handleAPK(this, stream)
        }
        val newId = notifyFinish(this)
        if (ForegroundTracker.hasForeground)
            onFinish(this, newId)
        if (!hasNotifications)
            stopSelf()
    }

    private fun ResponseBody.toProgressStream(subject: Subject): InputStream {
        val max = contentLength()
        val total = max.toFloat() / 1048576
        val id = subject.notifyID()

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

    // --- Notification managements

    fun Subject.notifyID() = hashCode()

    private fun notifyFail(subject: Subject) = lastNotify(subject.notifyID()) {
        broadcast(-2f, subject)
        it.setContentText(getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    private fun notifyFinish(subject: Subject) = lastNotify(subject.notifyID()) {
        broadcast(1f, subject)
        it.setIntent(subject)
            .setContentTitle(subject.title)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    private fun create() = Notifications.progress(this, "")

    fun update(id: Int, editor: (Notification.Builder) -> Unit = {}) {
        val wasEmpty = !hasNotifications
        val notification = notifications.getOrPut(id, ::create).also(editor)
        if (wasEmpty)
            updateForeground()
        else
            notify(id, notification.build())
    }

    private fun lastNotify(
        id: Int,
        editor: (Notification.Builder) -> Notification.Builder? = { null }
    ) : Int {
        val notification = remove(id)?.run(editor) ?: return -1
        val newId: Int = nextInt()
        notify(newId, notification.build())
        return newId
    }

    protected fun remove(id: Int) = notifications.remove(id)
        ?.also { updateForeground(); cancel(id) }
        ?: { cancel(id); null }()

    private fun notify(id: Int, notification: Notification) {
        Notifications.mgr.notify(id, notification)
    }

    private fun cancel(id: Int) {
        Notifications.mgr.cancel(id)
    }

    private fun updateForeground() {
        if (hasNotifications) {
            val (id, notification) = notifications.entries.first()
            startForeground(id, notification.build())
        } else {
            stopForeground(false)
        }
    }

    // --- Implement custom logic

    protected abstract suspend fun onFinish(subject: Subject, id: Int)

    protected abstract fun Notification.Builder.setIntent(subject: Subject): Notification.Builder

    // ---

    companion object {
        const val ACTION_KEY = "download_action"

        private val progressBroadcast = MutableLiveData<Pair<Float, Subject>?>()

        fun observeProgress(owner: LifecycleOwner, callback: (Float, Subject) -> Unit) {
            progressBroadcast.value = null
            progressBroadcast.observe(owner) {
                val (progress, subject) = it ?: return@observe
                callback(progress, subject)
            }
        }

        private fun broadcast(progress: Float, subject: Subject) {
            progressBroadcast.postValue(progress to subject)
        }
    }
}
