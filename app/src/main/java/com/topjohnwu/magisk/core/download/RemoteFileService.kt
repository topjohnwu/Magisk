package com.topjohnwu.magisk.core.download

import android.app.Notification
import android.content.Intent
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ForegroundTracker
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Magisk
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Module
import com.topjohnwu.superuser.ShellUtils
import kotlinx.coroutines.launch
import okhttp3.ResponseBody
import org.koin.android.ext.android.inject
import org.koin.core.KoinComponent
import timber.log.Timber
import java.io.IOException
import java.io.InputStream

abstract class RemoteFileService : NotificationService() {

    val service: GithubRawServices by inject()

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.getParcelableExtra<DownloadSubject>(ARG_URL)?.let {
            update(it.hashCode())
            coroutineScope.launch {
                try {
                    start(it)
                } catch (e: IOException) {
                    Timber.e(e)
                    failNotify(it)
                }
            }
        }
        return START_REDELIVER_INTENT
    }

    override fun createNotification() = Notifications.progress(this, "")

    // ---

    private suspend fun start(subject: DownloadSubject) {
        if (subject !is Magisk ||
            !subject.file.exists() ||
            !ShellUtils.checkSum("MD5", subject.file, subject.magisk.md5)) {
            val stream = service.fetchFile(subject.url).toProgressStream(subject)
            when (subject) {
                is Module ->
                    stream.toModule(subject.file, service.fetchInstaller().byteStream())
                else ->
                    stream.writeTo(subject.file)
            }
        }
        val newId = finishNotify(subject)
        if (ForegroundTracker.hasForeground) {
            onFinished(subject, newId)
        }
    }

    private fun ResponseBody.toProgressStream(subject: DownloadSubject): InputStream {
        val maxRaw = contentLength()
        val max = maxRaw / 1_000_000f
        val id = subject.hashCode()

        update(id) { it.setContentTitle(subject.title) }

        return ProgressInputStream(byteStream()) {
            val progress = it / 1_000_000f
            update(id) { notification ->
                if (maxRaw > 0) {
                    send(progress / max, subject)
                    notification
                        .setProgress(maxRaw.toInt(), it.toInt(), false)
                        .setContentText("%.2f / %.2f MB".format(progress, max))
                } else {
                    send(-1f, subject)
                    notification.setContentText("%.2f MB / ??".format(progress))
                }
            }
        }
    }

    private fun failNotify(subject: DownloadSubject) = lastNotify(subject.hashCode()) {
        send(0f, subject)
        it.setContentText(getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    private fun finishNotify(subject: DownloadSubject) = lastNotify(subject.hashCode()) {
        send(1f, subject)
        it.addActions(subject)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    // ---


    protected abstract suspend fun onFinished(subject: DownloadSubject, id: Int)

    protected abstract fun Notification.Builder.addActions(subject: DownloadSubject)
            : Notification.Builder

    companion object : KoinComponent {
        const val ARG_URL = "arg_url"

        private val internalProgressBroadcast = MutableLiveData<Pair<Float, DownloadSubject>>()
        val progressBroadcast: LiveData<Pair<Float, DownloadSubject>> get() = internalProgressBroadcast

        fun send(progress: Float, subject: DownloadSubject) {
            internalProgressBroadcast.postValue(progress to subject)
        }

        fun reset() {
            internalProgressBroadcast.value = null
        }
    }

}
