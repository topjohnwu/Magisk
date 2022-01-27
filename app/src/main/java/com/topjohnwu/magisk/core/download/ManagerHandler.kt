package com.topjohnwu.magisk.core.download

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.Intent
import androidx.core.content.getSystemService
import androidx.core.net.toFile
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ActivityTracker
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ktx.copyAndClose
import com.topjohnwu.magisk.ktx.writeTo
import java.io.File
import java.io.InputStream
import java.io.OutputStream

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

suspend fun DownloadService.handleAPK(subject: Subject.Manager, stream: InputStream) {
    fun write(output: OutputStream) {
        val external = subject.externalFile.outputStream()
        stream.copyAndClose(TeeOutputStream(external, output))
    }

    if (isRunningAsStub) {
        val apk = subject.file.toFile()
        val id = subject.notifyId
        write(DynAPK.update(this).outputStream())
        if (Info.stub!!.version < subject.stub.versionCode) {
            // Also upgrade stub
            update(id) {
                it.setProgress(0, 0, true)
                    .setContentTitle(getString(R.string.hide_app_title))
                    .setContentText("")
            }
            service.fetchFile(subject.stub.link).byteStream().writeTo(apk)
            val patched = File(apk.parent, "patched.apk")
            HideAPK.patch(this, apk, patched, packageName, applicationInfo.nonLocalizedLabel)
            apk.delete()
            patched.renameTo(apk)
        } else {
            val intent = packageManager.getLaunchIntentForPackage(packageName)
            intent!!.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            //noinspection InlinedApi
            val flag = PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
            val pending = PendingIntent.getActivity(this, id, intent, flag)
            if (ActivityTracker.hasForeground) {
                val alarm = getSystemService<AlarmManager>()
                alarm!!.set(AlarmManager.RTC, System.currentTimeMillis() + 1000, pending)
            }
            stopSelf()
            Runtime.getRuntime().exit(0)
        }
    } else {
        write(subject.file.outputStream())
    }
}
