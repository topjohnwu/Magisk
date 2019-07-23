package com.topjohnwu.magisk.model.download

import android.content.Intent
import androidx.core.app.NotificationCompat
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.FileRepository
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Magisk
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.Module
import com.topjohnwu.magisk.utils.ProgInputStream
import com.topjohnwu.magisk.utils.firstMap
import com.topjohnwu.magisk.utils.writeTo
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.ShellUtils
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import okhttp3.ResponseBody
import org.koin.android.ext.android.inject
import timber.log.Timber
import java.io.File
import java.io.InputStream

abstract class RemoteFileService : NotificationService() {

    private val repo by inject<FileRepository>()

    private val supportedFolders
        get() = listOfNotNull(
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
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess {
            runCatching { onFinished(it, subject) }.onFailure { Timber.e(it) }
            finish(it, subject)
        }.subscribeK()

    private fun search(subject: DownloadSubject) = Single.fromCallable {
        if (!Config.isDownloadCacheEnabled) {
            throw IllegalStateException("The download cache is disabled")
        }

        supportedFolders.firstMap { it.find(subject.file.name) }.also {
            if (subject is Magisk) {
                if (!ShellUtils.checkSum("MD5", it, subject.magisk.hash)) {
                    throw IllegalStateException("The given file doesn't match the hash")
                }
            }
        }
    }

    private fun download(subject: DownloadSubject) = repo.downloadFile(subject.url)
        .map { it.toStream(subject.hashCode()) }
        .map {
            subject.file.apply {
                when (subject) {
                    is Module -> it.toModule(this,
                            repo.downloadFile(Const.Url.MODULE_INSTALLER).blockingGet().byteStream())
                    else -> it.writeTo(this)
                }
            }
        }

    // ---

    private fun File.find(name: String) = list().orEmpty()
        .firstOrNull { it == name }
        ?.let { File(this, it) }

    private fun ResponseBody.toStream(id: Int): InputStream {
        val maxRaw = contentLength()
        val max = maxRaw / 1_000_000f

        return ProgInputStream(byteStream()) {
            val progress = it / 1_000_000f

            update(id) { notification ->
                notification
                    .setProgress(maxRaw.toInt(), it.toInt(), false)
                    .setContentText(getString(R.string.download_progress, progress, max))
            }
        }
    }

    private fun finish(file: File, subject: DownloadSubject) = finishWork(subject.hashCode()) {
        it.addActions(file, subject)
            .setContentText(getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
    }

    // ---


    @Throws(Throwable::class)
    protected abstract fun onFinished(file: File, subject: DownloadSubject)

    protected abstract fun NotificationCompat.Builder.addActions(
        file: File,
        subject: DownloadSubject
    ): NotificationCompat.Builder

    companion object {
        const val ARG_URL = "arg_url"
    }

}