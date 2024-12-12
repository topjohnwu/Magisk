package com.topjohnwu.magisk.core.download

import android.Manifest
import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.app.job.JobInfo
import android.app.job.JobScheduler
import android.content.Context
import android.net.Uri
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.collection.SparseArrayCompat
import androidx.collection.isNotEmpty
import androidx.core.content.getSystemService
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.MutableLiveData
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.JobService
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.base.IActivityExtension
import com.topjohnwu.magisk.core.cmp
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.cachedFile
import com.topjohnwu.magisk.core.ktx.copyAll
import com.topjohnwu.magisk.core.ktx.copyAndClose
import com.topjohnwu.magisk.core.ktx.set
import com.topjohnwu.magisk.core.ktx.withInOut
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.magisk.view.Notifications
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import okhttp3.ResponseBody
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream
import org.apache.commons.compress.archivers.zip.ZipFile
import timber.log.Timber
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream

/**
 * This class drives the execution of file downloads and notification management.
 *
 * Each download engine instance has to be paired with a "session" that is managed by the operating
 * system. A session is an Android component that allows executing long lasting operations and
 * have its state tied to a notification to show progress.
 *
 * A session can only have one single notification representing its state, and the operating system
 * also uses the notification to manage the lifecycle of a session. One goal of this class is
 * to support concurrent download tasks using only one single session, so internally it manages
 * all active tasks and notifications and properly re-assign notifications to be attached to
 * the session to make sure all download operations can be completed without the operating system
 * killing the session.
 *
 * For API 23 - 33, we use a foreground service as a session.
 * For API 34 and higher, we use user-initiated job services as a session.
 */
class DownloadEngine(
    private val session: Session
) {

    interface Session {
        val context: Context

        fun attachNotification(id: Int, builder: Notification.Builder)
        fun onDownloadComplete()
    }

    companion object {
        const val ACTION = "com.topjohnwu.magisk.DOWNLOAD"
        const val SUBJECT_KEY = "subject"
        private const val REQUEST_CODE = 1

        private val progressBroadcast = MutableLiveData<Pair<Float, Subject>?>()

        private fun broadcast(progress: Float, subject: Subject) {
            progressBroadcast.postValue(progress to subject)
        }

        fun observeProgress(owner: LifecycleOwner, callback: (Float, Subject) -> Unit) {
            progressBroadcast.value = null
            progressBroadcast.observe(owner) {
                val (progress, subject) = it ?: return@observe
                callback(progress, subject)
            }
        }

        private fun createBroadcastIntent(context: Context, subject: Subject) =
            context.intent<com.topjohnwu.magisk.core.Receiver>()
                .setAction(ACTION)
                .putExtra(SUBJECT_KEY, subject)

        private fun createServiceIntent(context: Context, subject: Subject) =
            context.intent<com.topjohnwu.magisk.core.Service>()
                .setAction(ACTION)
                .putExtra(SUBJECT_KEY, subject)

        @SuppressLint("InlinedApi")
        fun getPendingIntent(context: Context, subject: Subject): PendingIntent {
            val flag = PendingIntent.FLAG_IMMUTABLE or
                PendingIntent.FLAG_UPDATE_CURRENT or
                PendingIntent.FLAG_ONE_SHOT
            return if (Build.VERSION.SDK_INT >= 34) {
                // On API 34+, download tasks are handled with a user-initiated job.
                // However, there is no way to schedule a new job directly with a pending intent.
                // As a workaround, we send the subject to a broadcast receiver and have it
                // schedule the job for us.
                val intent = createBroadcastIntent(context, subject)
                PendingIntent.getBroadcast(context, REQUEST_CODE, intent, flag)
            } else {
                val intent = createServiceIntent(context, subject)
                if (Build.VERSION.SDK_INT >= 26) {
                    PendingIntent.getForegroundService(context, REQUEST_CODE, intent, flag)
                } else {
                    PendingIntent.getService(context, REQUEST_CODE, intent, flag)
                }
            }
        }

        @SuppressLint("InlinedApi")
        fun <T> startWithActivity(
            activity: T,
            subject: Subject
        ) where T : ComponentActivity, T : IActivityExtension {
            activity.withPermission(Manifest.permission.POST_NOTIFICATIONS) {
                // Always download regardless of notification permission status
                start(activity.applicationContext, subject)
            }
        }

        fun start(context: Context, subject: Subject) {
            if (Build.VERSION.SDK_INT >= 34) {
                val scheduler = context.getSystemService<JobScheduler>()!!
                val cmp = JobService::class.java.cmp(context.packageName)
                val extras = Bundle()
                extras.putParcelable(SUBJECT_KEY, subject)
                val info = JobInfo.Builder(Const.ID.DOWNLOAD_JOB_ID, cmp)
                    .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                    .setUserInitiated(true)
                    .setTransientExtras(extras)
                    .build()
                scheduler.schedule(info)
            } else {
                val intent = createServiceIntent(context, subject)
                if (Build.VERSION.SDK_INT >= 26) {
                    context.startForegroundService(intent)
                } else {
                    context.startService(intent)
                }
            }
        }
    }

    fun download(subject: Subject) {
        notifyUpdate(subject.notifyId)
        CoroutineScope(job + Dispatchers.IO).launch {
            try {
                val stream = network.fetchFile(subject.url).toProgressStream(subject)
                when (subject) {
                    is Subject.App -> handleApp(stream, subject)
                    is Subject.Module -> handleModule(stream, subject.file)
                    else -> stream.copyAndClose(subject.file.outputStream())
                }
                val activity = AppContext.foregroundActivity
                if (activity != null && subject.autoLaunch) {
                    notifyRemove(subject.notifyId)
                    subject.pendingIntent(activity)?.send()
                } else {
                    notifyFinish(subject)
                }
            } catch (e: Exception) {
                Timber.e(e)
                notifyFail(subject)
            }
        }
    }

    @Synchronized
    fun reattach() {
        val builder = notifications[attachedId] ?: return
        session.attachNotification(attachedId, builder)
    }

    private val notifications = SparseArrayCompat<Notification.Builder>()
    private var attachedId = -1

    private val job = Job()

    private val context get() = session.context
    private val network get() = ServiceLocator.networkService

    private fun finalNotify(id: Int, editor: (Notification.Builder) -> Unit): Int {
        val notification = notifyRemove(id)?.also(editor) ?: return -1
        val newId = Notifications.nextId()
        Notifications.mgr.notify(newId, notification.build())
        return newId
    }

    private fun notifyFail(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(-2f, subject)
        it.setContentText(context.getString(R.string.download_file_error))
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setOngoing(false)
    }

    private fun notifyFinish(subject: Subject) = finalNotify(subject.notifyId) {
        broadcast(1f, subject)
        it.setContentTitle(subject.title)
            .setContentText(context.getString(R.string.download_complete))
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setProgress(0, 0, false)
            .setOngoing(false)
            .setAutoCancel(true)
        subject.pendingIntent(context)?.let { intent -> it.setContentIntent(intent) }
    }

    private fun attachNotification(id: Int, notification: Notification.Builder) {
        attachedId = id
        session.attachNotification(id, notification)
    }

    @Synchronized
    private fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit = {}) {
        val notification = (notifications[id] ?: Notifications.startProgress("").also {
            notifications[id] = it
        }).apply(editor)

        if (attachedId < 0)
            attachNotification(id, notification)
        else
            Notifications.mgr.notify(id, notification.build())
    }

    @Synchronized
    private fun notifyRemove(id: Int): Notification.Builder? {
        val idx = notifications.indexOfKey(id)
        var n: Notification.Builder? = null

        if (idx >= 0) {
            n = notifications.valueAt(idx)
            notifications.removeAt(idx)

            // The cancelled notification is the one attached to the session, need special handling
            if (attachedId == id) {
                if (notifications.isNotEmpty()) {
                    // There are still remaining notifications, pick one and attach to the session
                    val anotherId = notifications.keyAt(0)
                    val notification = notifications.valueAt(0)
                    attachNotification(anotherId, notification)
                } else {
                    // No more notifications left, terminate the session
                    attachedId = -1
                    session.onDownloadComplete()
                }
            }
        }

        Notifications.mgr.cancel(id)
        return n
    }

    private suspend fun handleApp(stream: InputStream, subject: Subject.App) {
        val external = subject.file.outputStream()

        if (isRunningAsStub) {
            val updateApk = StubApk.update(context)
            try {
                // Download full APK to stub update path
                stream.copyAndClose(TeeOutputStream(external, updateApk.outputStream()))

                // Also upgrade stub
                notifyUpdate(subject.notifyId) {
                    it.setProgress(0, 0, true)
                        .setContentTitle(context.getString(R.string.hide_app_title))
                        .setContentText("")
                }

                // Extract stub
                val apk = context.cachedFile("stub.apk")
                ZipFile.Builder().setFile(updateApk).get().use { zf ->
                    apk.delete()
                    zf.getInputStream(zf.getEntry("assets/stub.apk")).writeTo(apk)
                }

                // Patch and install
                subject.intent = AppMigration.upgradeStub(context, apk)
                    ?: throw IOException("HideAPK patch error")
                apk.delete()
            } catch (e: Exception) {
                // If any error occurred, do not let stub load the new APK
                updateApk.delete()
                throw e
            }
        } else {
            val session = APKInstall.startSession(context)
            stream.copyAndClose(TeeOutputStream(external, session.openStream(context)))
            subject.intent = session.waitIntent()
        }
    }

    private suspend fun handleModule(src: InputStream, file: Uri) {
        val tmp = context.cachedFile("module.zip")
        try {
            // First download the entire zip into cache so we can process it
            src.writeTo(tmp)

            val input = ZipFile.Builder().setFile(tmp).get()
            val output = ZipArchiveOutputStream(file.outputStream())
            withInOut(input, output) { zin, zout ->
                zout.putArchiveEntry(ZipArchiveEntry("META-INF/"))
                zout.closeArchiveEntry()
                zout.putArchiveEntry(ZipArchiveEntry("META-INF/com/"))
                zout.closeArchiveEntry()
                zout.putArchiveEntry(ZipArchiveEntry("META-INF/com/google/"))
                zout.closeArchiveEntry()
                zout.putArchiveEntry(ZipArchiveEntry("META-INF/com/google/android/"))
                zout.closeArchiveEntry()

                zout.putArchiveEntry(ZipArchiveEntry("META-INF/com/google/android/update-binary"))
                context.assets.open("module_installer.sh").use { it.copyAll(zout) }
                zout.closeArchiveEntry()

                zout.putArchiveEntry(ZipArchiveEntry("META-INF/com/google/android/updater-script"))
                zout.write("#MAGISK\n".toByteArray())
                zout.closeArchiveEntry()

                // Then simply copy all entries to output
                zin.copyRawEntries(zout) { entry -> !entry.name.startsWith("META-INF") }
            }
        } finally {
            tmp.delete()
        }
    }

    private class TeeOutputStream(
        private val o1: OutputStream,
        private val o2: OutputStream
    ) : OutputStream() {
        override fun write(b: Int) {
            o1.write(b)
            o2.write(b)
        }
        override fun write(b: ByteArray?, off: Int, len: Int) {
            o1.write(b, off, len)
            o2.write(b, off, len)
        }
        override fun close() {
            o1.close()
            o2.close()
        }
    }

    private fun ResponseBody.toProgressStream(subject: Subject): InputStream {
        val max = contentLength()
        val total = max.toFloat() / 1048576
        val id = subject.notifyId

        notifyUpdate(id) { it.setContentTitle(subject.title) }

        return ProgressInputStream(byteStream()) {
            val progress = it.toFloat() / 1048576
            notifyUpdate(id) { notification ->
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
}
