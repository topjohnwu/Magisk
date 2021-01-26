package com.topjohnwu.magisk.core.download

import android.content.Context
import androidx.core.net.toFile
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.ktx.relaunchApp
import com.topjohnwu.magisk.ktx.writeTo
import java.io.File

private fun Context.patch(apk: File) {
    val patched = File(apk.parent, "patched.apk")
    HideAPK.patch(this, apk, patched, packageName, applicationInfo.nonLocalizedLabel)
    apk.delete()
    patched.renameTo(apk)
}

private fun BaseDownloader.notifyHide(id: Int) {
    update(id) {
        it.setProgress(0, 0, true)
            .setContentTitle(getString(R.string.hide_manager_title))
            .setContentText("")
    }
}

suspend fun BaseDownloader.handleAPK(subject: Subject.Manager) {
    if (!isRunningAsStub)
        return
    val apk = subject.file.toFile()
    val id = subject.notifyID()
    // Move to upgrade location
    apk.copyTo(DynAPK.update(this), overwrite = true)
    apk.delete()
    if (Info.stub!!.version < subject.stub.versionCode) {
        notifyHide(id)
        // Also upgrade stub
        service.fetchFile(subject.stub.link).byteStream().writeTo(apk)
        patch(apk)
    } else {
        // Simply relaunch the app
        stopSelf()
        relaunchApp(this)
    }
}
