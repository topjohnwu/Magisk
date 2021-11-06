package com.topjohnwu.magisk.core.download

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import androidx.core.net.toFile
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.core.ForegroundTracker
import com.topjohnwu.magisk.core.base.BaseService
import com.topjohnwu.magisk.core.download.Action.Flash
import com.topjohnwu.magisk.core.download.Subject.Manager
import com.topjohnwu.magisk.core.download.Subject.Module
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.internal.UiThreadHandler
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

class DownloadService : BaseService() {

    private val context get() = this
    private val hasNotifications get() = notifications.isNotEmpty()
    private val notifications = Collections.synchronizedMap(HashMap<Int, Notification.Builder>())
    private val coroutineScope = CoroutineScope(Dispatchers.IO)

    val service get() = ServiceLocator.networkService
    private val mgr get() = Notifications.mgr

    // -- Service overrides

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        intent.getParcelableExtra<Subject>(SUBJECT_KEY)?.let { subject ->
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
        notifications.forEach { mgr.cancel(it.key) }
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
            is Module ->  // Download and process on-the-fly
                stream.toModule(file, service.fetchInstaller().byteStream())
            is Manager -> handleAPK(this, stream)
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

    // --- Notifications

    private fun notifyFail(subject: Subject) = finalNotify(subject.notifyID()) {
        broadcast(-2f, subject)
        it.setContentText(getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    private fun notifyFinish(subject: Subject) = finalNotify(subject.notifyID()) {
        broadcast(1f, subject)
        it.setIntent(subject)
            .setContentTitle(subject.title)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    private fun finalNotify(
        id: Int,
        editor: (Notification.Builder) -> Notification.Builder
    ) : Int {
        val notification = remove(id)?.run(editor) ?: return -1
        val newId = nextInt()
        mgr.notify(newId, notification.build())
        return newId
    }

    fun create() = Notifications.progress(this, "")

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

    private fun Notification.Builder.setIntent(subject: Subject) = when (subject) {
        is Module -> setIntent(subject)
        is Manager -> setIntent(subject)
    }

    private fun Notification.Builder.setIntent(subject: Module)
            = when (subject.action) {
        Flash -> setContentIntent(FlashFragment.installIntent(context, subject.file))
        else -> setContentIntent(Intent())
    }

    private fun Notification.Builder.setIntent(subject: Manager) =
        setContentIntent(APKInstall.installIntent(context, subject.file.toFile()))

    @SuppressLint("InlinedApi")
    private fun Notification.Builder.setContentIntent(intent: Intent) =
        setContentIntent(PendingIntent.getActivity(context, nextInt(), intent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_ONE_SHOT))

    // -- Post download processing

    private fun onFinish(subject: Subject, id: Int) = when (subject) {
        is Module -> subject.onFinish(id)
        is Manager -> subject.onFinish(id)
    }

    private fun Module.onFinish(id: Int) = when (action) {
        Flash -> {
            UiThreadHandler.run {
                (ForegroundTracker.foreground as? BaseUIActivity<*, *>)
                    ?.navigation?.navigate(FlashFragment.install(file, id))
            }
        }
        else -> Unit
    }

    private fun Manager.onFinish(id: Int) {
        remove(id)
        APKInstall.install(context, file.toFile())
    }

    companion object {
        private const val SUBJECT_KEY = "download_subject"

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
        fun pendingIntent(context: Context, subject: Subject): PendingIntent {
            return if (Build.VERSION.SDK_INT >= 26) {
                PendingIntent.getForegroundService(context, nextInt(), intent(context, subject),
                    PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT)
            } else {
                PendingIntent.getService(context, nextInt(), intent(context, subject),
                    PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT)
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
