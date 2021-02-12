package com.topjohnwu.magisk.core.download

import android.content.Context
import androidx.core.net.toFile
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ktx.relaunchApp
import com.topjohnwu.magisk.ktx.withStreams
import com.topjohnwu.magisk.ktx.writeTo
import java.io.File
import java.io.InputStream
import java.io.OutputStream

private fun Context.patch(apk: File) {
    val patched = File(apk.parent, "patched.apk")
    HideAPK.patch(this, apk, patched, packageName, applicationInfo.nonLocalizedLabel)
    apk.delete()
    patched.renameTo(apk)
}

private fun BaseDownloader.notifyHide(id: Int) {
    update(id) {
        it.setProgress(0, 0, true)
            .setContentTitle(getString(R.string.hide_app_title))
            .setContentText("")
    }
}

private class DupOutputStream(
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

suspend fun BaseDownloader.handleAPK(subject: Subject.Manager, stream: InputStream) {
    fun write(output: OutputStream) {
        val ext = subject.externalFile.outputStream()
        val o = DupOutputStream(ext, output)
        withStreams(stream, o) { src, out -> src.copyTo(out) }
    }

    if (isRunningAsStub) {
        val apk = subject.file.toFile()
        val id = subject.notifyID()
        write(DynAPK.update(this).outputStream())
        if (Info.stub!!.version < subject.stub.versionCode) {
            // Also upgrade stub
            notifyHide(id)
            service.fetchFile(subject.stub.link).byteStream().writeTo(apk)
            patch(apk)
        } else {
            // Simply relaunch the app
            stopSelf()
            relaunchApp(this)
        }
    } else {
        write(subject.file.outputStream())
    }
}
