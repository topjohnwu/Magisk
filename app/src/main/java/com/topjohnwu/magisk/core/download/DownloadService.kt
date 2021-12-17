package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.app.PendingIntent.*
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ForegroundTracker
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.copyAndClose
import com.topjohnwu.magisk.ktx.synchronized
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.magisk.view.Notifications.mgr
import kotlinx.coroutines.*
import okhttp3.ResponseBody
import timber.log.Timber
import java.io.IOException
import java.io.InputStream
import java.util.*
import kotlin.collections.HashMap

class DownloadService : BaseService() {

    private val hasNotifications get() = notifications.isNotEmpty()
    private val notifications = HashMap<Int, Notification.Builder>().synchronized()
    private val job = Job()

    val service get() = ServiceLocator.networkService

    // -- Service overrides

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        intent.getParcelableExtra<Subject>(SUBJECT_KEY)?.let { doDownload(it) }
        return START_NOT_STICKY
    }

    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        notifications.forEach { mgr.cancel(it.key) }
        notifications.clear()
    }

    override fun onDestroy() {
        super.onDestroy()
        job.cancel()
    }

    // -- Download logic

    private fun doDownload(subject: Subject) {
        update(subject.notifyId)
        val coroutineScope = CoroutineScope(job + Dispatchers.IO)
        coroutineScope.launch {
            try {
                val stream = service.fetchFile(subject.url).toProgressStream(subject)
                when (subject) {
                    is Subject.Manager -> handleAPK(subject, stream)
                    else -> stream.copyAndClose(subject.file.outputStream())
                }
                if (ForegroundTracker.hasForeground) {
                    remove(subject.notifyId)
                    subject.pendingIntent(this@DownloadService).send()
                } else {
                    notifyFinish(subject)
                }
                if (!hasNotifications)
                    stopSelf()
            } catch (e: IOException) {
                Timber.e(e)
                notifyFail(subject)
            }
        }
    }

    private fun ResponseBody.toProgressStream(subject: Subject): InputStream {
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

    // --- Notification management

    private fun notifyFail(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(-2f, subject)
        it.setContentText(getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    private fun notifyFinish(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(1f, subject)
        it.setContentIntent(subject.pendingIntent(this))
            .setContentTitle(subject.title)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    private fun finalNotify(id: Int, editor: (Notification.Builder) -> Unit): Int {
        val notification = remove(id)?.also(editor) ?: return -1
        val newId = Notifications.nextId()
        mgr.notify(newId, notification.build())
        return newId
    }

    private fun create() = Notifications.progress(this, "")

    fun update(id: Int, editor: (Notification.Builder) -> Unit = {}) {
        val wasEmpty = !hasNotifications
        val notification = notifications.getOrPut(id, ::create).also(editor)
        if (wasEmpty)
            updateForeground()
        else
            mgr.notify(id, notification.build())
    }

    private fun remove(id: Int): Notification.Builder? {
        val n = notifications.remove(id)?.also { updateForeground() }
        mgr.cancel(id)
        return n
    }

    private fun updateForeground() {
        if (hasNotifications) {
            val (id, notification) = notifications.entries.first()
            startForeground(id, notification.build())
        } else {
            stopForeground(false)
        }
    }

    companion object {
        private const val SUBJECT_KEY = "download_subject"
        private const val REQUEST_CODE = 1

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

        private fun intent(context: Context, subject: Subject) =
            context.intent<DownloadService>().putExtra(SUBJECT_KEY, subject)

        @SuppressLint("InlinedApi")
        fun getPendingIntent(context: Context, subject: Subject): PendingIntent {
            val flag = FLAG_IMMUTABLE or FLAG_UPDATE_CURRENT or FLAG_ONE_SHOT
            val intent = intent(context, subject)
            return if (Build.VERSION.SDK_INT >= 26) {
                getForegroundService(context, REQUEST_CODE, intent, flag)
            } else {
                getService(context, REQUEST_CODE, intent, flag)
            }
        }

        fun start(context: Context, subject: Subject) {
            val app = context.applicationContext
            if (Build.VERSION.SDK_INT >= 26) {
                app.startForegroundService(intent(app, subject))
            } else {
                app.startService(intent(app, subject))
            }
        }
    }

}
