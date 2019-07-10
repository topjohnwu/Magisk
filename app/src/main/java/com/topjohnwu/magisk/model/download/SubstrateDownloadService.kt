package com.topjohnwu.magisk.model.download

import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.webkit.MimeTypeMap
import android.widget.Toast
import androidx.core.app.NotificationCompat
import androidx.core.content.getSystemService
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.FileRepository
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.utils.provide
import com.topjohnwu.magisk.utils.toast
import com.topjohnwu.magisk.utils.writeToCachedFile
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.ShellUtils
import io.reactivex.Single
import okhttp3.ResponseBody
import org.koin.android.ext.android.inject
import java.io.File
import kotlin.random.Random.Default.nextInt

abstract class SubstrateDownloadService : Service() {

    private var _notification: NotificationCompat.Builder? = null

    private val repo by inject<FileRepository>()

    private val notification: NotificationCompat.Builder
        get() = _notification ?: Notifications.progress(this, "")
            .setContentText(getString(R.string.download_local))
            .also { _notification = it }

    override fun onBind(p0: Intent?): IBinder? = null

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.getParcelableExtra<DownloadSubject>(ARG_URL)?.let {
            updateNotification { notification -> notification.setContentTitle(it.fileName) }
            start(it)
        }
        return START_REDELIVER_INTENT
    }

    // ---

    private fun start(subject: DownloadSubject) = search(subject)
        .onErrorResumeNext(download(subject))
        .subscribeK {
            runCatching { onFinished(it, subject) }
            finish(it, subject)
        }

    private fun search(subject: DownloadSubject) = Single.fromCallable {
        if (!Config.isDownloadCacheEnabled) {
            throw IllegalStateException("The download cache is disabled")
        }

        val file = runCatching {
            cacheDir.list().orEmpty()
                .first { it == subject.fileName } // this throws an exception if not found
                .let { File(cacheDir, it) }
        }.getOrElse {
            Const.EXTERNAL_PATH.list().orEmpty()
                .first { it == subject.fileName } // this throws an exception if not found
                .let { File(Const.EXTERNAL_PATH, it) }
        }

        if (subject is DownloadSubject.Magisk) {
            if (!ShellUtils.checkSum("MD5", file, subject.magisk.hash)) {
                throw IllegalStateException("The given file doesn't match the hash")
            }
        }

        file
    }

    private fun download(subject: DownloadSubject) = repo.downloadFile(subject.url)
        .map { it.toFile(subject.fileName) }

    // ---

    protected fun fileIntent(fileName: String): Intent {
        val file = downloadsFile(fileName)
        return Intent(Intent.ACTION_VIEW)
            .setDataAndType(file.provide(this), file.type)
            .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
    }

    protected fun moveToDownloads(file: File) {
        val destination = downloadsFile(file.name)
        file.copyTo(destination)
        toast(getString(R.string.internal_storage, "/Download/${file.name}"), Toast.LENGTH_LONG)
    }

    // ---

    private val File.type
        get() = MimeTypeMap.getSingleton()
            .getMimeTypeFromExtension(extension)
            .orEmpty()

    private fun downloadsFile(name: String) = File(Const.EXTERNAL_PATH, name)

    private fun ResponseBody.toFile(name: String): File {
        val maxRaw = contentLength()
        val max = maxRaw / 1_000_000f

        return writeToCachedFile(this@SubstrateDownloadService, name) {
            val progress = it / 1_000_000f

            updateNotification { notification ->
                notification
                    .setProgress(maxRaw.toInt(), it.toInt(), false)
                    .setContentText(getString(R.string.download_progress, progress, max))
            }
        }
    }

    private fun finish(file: File, subject: DownloadSubject) {
        stopForeground(false)

        val notification = notification.addActions(file, subject)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
            .build()

        getSystemService<NotificationManager>()?.notify(nextInt(), notification)

        stopSelf()
    }

    private inline fun updateNotification(body: (NotificationCompat.Builder) -> Unit = {}) {
        startForeground(ID, notification.also(body).build())
    }

    // ---


    @Throws(Throwable::class)
    protected abstract fun onFinished(file: File, subject: DownloadSubject)

    protected abstract fun NotificationCompat.Builder.addActions(
        file: File,
        subject: DownloadSubject
    ): NotificationCompat.Builder


    companion object {
        private const val ID = 300
        const val ARG_URL = "arg_url"
    }

}