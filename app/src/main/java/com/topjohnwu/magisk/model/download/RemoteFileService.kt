package com.topjohnwu.magisk.model.download

import android.app.Activity
import android.content.Intent
import androidx.core.app.NotificationCompat
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.di.NullActivity
import com.topjohnwu.magisk.extensions.firstMap
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.*
import com.topjohnwu.magisk.utils.ProgressInputStream
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.ShellUtils
import io.reactivex.Single
import okhttp3.ResponseBody
import org.koin.android.ext.android.inject
import timber.log.Timber
import java.io.File
import java.io.InputStream

abstract class RemoteFileService : NotificationService() {

    private val service: GithubRawServices by inject()

    private val supportedFolders
        get() = listOf(
            cacheDir,
            Config.downloadDirectory
        )

    override val defaultNotification: NotificationCompat.Builder
        get() = Notifications
            .progress(this, "")
            .setContentText(getString(R.string.download_local))

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.getParcelableExtra<DownloadSubject>(ARG_URL)?.let { start(it) }
        return START_REDELIVER_INTENT
    }

    // ---

    private fun start(subject: DownloadSubject) = search(subject)
        .onErrorResumeNext(download(subject))
        .doOnSubscribe { update(subject.hashCode()) { it.setContentTitle(subject.title) } }
        .subscribeK(onError = {
            Timber.e(it)
            finishNotify(subject.hashCode()) { notification ->
                notification.setContentText(getString(R.string.download_file_error))
                        .setSmallIcon(android.R.drawable.stat_notify_error)
                        .setOngoing(false)
            }
        }) {
            val newId = finishNotify(it, subject)
            if (get<Activity>() !is NullActivity) {
                onFinished(it, subject, newId)
            }
        }

    private fun search(subject: DownloadSubject) = Single.fromCallable {
        if (!Config.isDownloadCacheEnabled || subject is Manager) {
            throw IllegalStateException("The download cache is disabled")
        }

        supportedFolders.firstMap { it.find(subject.file.name) }.also {
            if (subject is Magisk) {
                if (!ShellUtils.checkSum("MD5", it, subject.magisk.md5)) {
                    throw IllegalStateException("The given file doesn't match the md5")
                }
            }
        }
    }

    private fun download(subject: DownloadSubject) = service.fetchFile(subject.url)
        .map { it.toStream(subject.hashCode()) }
        .flatMap { stream ->
            when (subject) {
                is Module -> service.fetchInstaller()
                        .map { stream.toModule(subject.file, it.byteStream()); subject.file }
                else -> Single.fromCallable { stream.writeTo(subject.file); subject.file }
            }
        }.map {
            when (subject) {
                is Manager -> handleAPK(it, subject)
                else -> it
            }
        }

    // ---

    private fun File.find(name: String) = list().orEmpty()
        .firstOrNull { it == name }
        ?.let { File(this, it) }

    private fun ResponseBody.toStream(id: Int): InputStream {
        val maxRaw = contentLength()
        val max = maxRaw / 1_000_000f

        return ProgressInputStream(byteStream()) {
            val progress = it / 1_000_000f
            update(id) { notification ->
                notification
                    .setProgress(maxRaw.toInt(), it.toInt(), false)
                    .setContentText(getString(R.string.download_progress, progress, max))
            }
        }
    }

    private fun finishNotify(file: File, subject: DownloadSubject) = finishNotify(subject.hashCode()) {
        it.addActions(file, subject)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    // ---


    @Throws(Throwable::class)
    protected abstract fun onFinished(file: File, subject: DownloadSubject, id: Int)

    protected abstract fun NotificationCompat.Builder.addActions(
        file: File,
        subject: DownloadSubject
    ): NotificationCompat.Builder

    companion object {
        const val ARG_URL = "arg_url"
    }

}