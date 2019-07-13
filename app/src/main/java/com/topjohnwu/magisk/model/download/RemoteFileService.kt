package com.topjohnwu.magisk.model.download

import android.content.Intent
import androidx.core.app.NotificationCompat
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.FileRepository
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject.*
import com.topjohnwu.magisk.utils.firstMap
import com.topjohnwu.magisk.utils.writeToCachedFile
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.ShellUtils
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import okhttp3.ResponseBody
import org.koin.android.ext.android.get
import org.koin.android.ext.android.inject
import org.koin.core.parameter.parametersOf
import timber.log.Timber
import java.io.File

abstract class RemoteFileService : NotificationService() {

    private val repo by inject<FileRepository>()

    private val supportedFolders
        get() = listOfNotNull(
            cacheDir,
            Config.downloadsFile(),
            Const.EXTERNAL_PATH
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

    private fun startInternal(subject: DownloadSubject): Single<File> = search(subject)
        .onErrorResumeNext(download(subject))
        .doOnSubscribe { update(subject.hashCode()) { it.setContentTitle(subject.fileName) } }
        .observeOn(AndroidSchedulers.mainThread())
        .doOnSuccess {
            runCatching { onFinished(it, subject) }.onFailure { Timber.e(it) }
            finish(it, subject)
        }

    private fun start(subject: DownloadSubject) = startInternal(subject).subscribeK()

    private fun search(subject: DownloadSubject) = Single.fromCallable {
        if (!Config.isDownloadCacheEnabled) {
            throw IllegalStateException("The download cache is disabled")
        }

        val file = supportedFolders.firstMap { it.find(subject.fileName) }

        if (subject is Magisk) {
            if (!ShellUtils.checkSum("MD5", file, subject.magisk.hash)) {
                throw IllegalStateException("The given file doesn't match the hash")
            }
        }

        file
    }

    private fun download(subject: DownloadSubject) = repo.downloadFile(subject.url)
        .map { it.toFile(subject.hashCode(), subject.fileName) }
        .map {
            when (subject) {
                is Module -> {
                    update(subject.hashCode()) {
                        it.setContentText(getString(R.string.download_module))
                            .setProgress(0, 0, true)
                    }

                    get<ModuleTransformer> { parametersOf(subject) }
                        .inject(it, startInternal(Installer).blockingGet())
                }
                else -> it
            }
        }

    // ---

    private fun File.find(name: String) = list().orEmpty()
        .firstOrNull { it == name }
        ?.let { File(this, it) }

    private fun ResponseBody.toFile(id: Int, name: String): File {
        val maxRaw = contentLength()
        val max = maxRaw / 1_000_000f

        return writeToCachedFile(this@RemoteFileService, name) {
            val progress = it / 1_000_000f

            update(id) { notification ->
                notification
                    .setProgress(maxRaw.toInt(), it.toInt(), false)
                    .setContentText(getString(R.string.download_progress, progress, max))
            }
        }
    }

    private fun finish(file: File, subject: DownloadSubject) = finishWork(subject.hashCode()) {
        if (subject is Installer) return@finishWork null

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