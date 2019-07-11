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
import com.topjohnwu.magisk.utils.cachedFile
import com.topjohnwu.magisk.utils.withStreams
import com.topjohnwu.magisk.utils.writeToCachedFile
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.ShellUtils
import io.reactivex.Single
import io.reactivex.android.schedulers.AndroidSchedulers
import okhttp3.ResponseBody
import org.koin.android.ext.android.inject
import timber.log.Timber
import java.io.File
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

abstract class RemoteFileService : NotificationService() {

    private val repo by inject<FileRepository>()

    override val defaultNotification: NotificationCompat.Builder
        get() = Notifications
            .progress(this, "")
            .setContentText(getString(R.string.download_local))

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.getParcelableExtra<DownloadSubject>(ARG_URL)?.let { start(it) }
        return START_REDELIVER_INTENT
    }

    // ---

    private fun startInternal(subject: DownloadSubject) = search(subject)
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

        val file = runCatching {
            cacheDir.list().orEmpty()
                .first { it == subject.fileName } // this throws an exception if not found
                .let { File(cacheDir, it) }
        }.getOrElse {
            Const.EXTERNAL_PATH.list().orEmpty()
                .first { it == subject.fileName } // this throws an exception if not found
                .let { File(Const.EXTERNAL_PATH, it) }
        }

        if (subject is Magisk) {
            if (!ShellUtils.checkSum("MD5", file, subject.magisk.hash)) {
                throw IllegalStateException("The given file doesn't match the hash")
            }
        }

        file
    }

    private fun download(subject: DownloadSubject) = repo.downloadFile(subject.url)
        .map {
            when (subject) {
                is Module -> appendInstaller(it, subject)
                else -> it.toFile(subject.hashCode(), subject.fileName)
            }
        }

    private fun appendInstaller(body: ResponseBody, subject: DownloadSubject): File {
        update(subject.hashCode()) {
            it.setContentText(getString(R.string.download_module))
        }

        val installer = startInternal(Installer).blockingGet()
        val target = cachedFile(subject.fileName)

        val input = ZipInputStream(body.byteStream())
        val output = ZipOutputStream(target.outputStream())

        withStreams(input, output) { zin, zout ->
            zout.putNextEntry(ZipEntry("META-INF/"))
            zout.putNextEntry(ZipEntry("META-INF/com/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/android/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/android/update-binary"))
            installer.inputStream().copyTo(zout).also { zout.flush() }

            zout.putNextEntry(ZipEntry("META-INF/com/google/android/updater-script"))
            zout.write("#MAGISK\n".toByteArray(charset("UTF-8")))

            var off = -1
            var entry: ZipEntry? = zin.nextEntry
            while (entry != null) {
                Timber.i("Let's gooo (${entry.name})")
                if (off < 0) {
                    off = entry.name.indexOf('/') + 1
                }

                val path = entry.name.substring(off)
                if (path.isNotEmpty() && !path.startsWith("META-INF")) {
                    zout.putNextEntry(ZipEntry(path))
                    if (!entry.isDirectory) {
                        zin.copyTo(zout).also { zout.flush() }
                    }
                }

                entry = zin.nextEntry
            }
        }

        return target
    }

    // ---

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

    protected abstract fun map(subject: DownloadSubject, file: File): File

    companion object {
        const val ARG_URL = "arg_url"
    }

}