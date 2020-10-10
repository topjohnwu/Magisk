package com.topjohnwu.magisk.core.download

import android.content.Context
import androidx.core.net.toFile
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Action.APK.Restore
import com.topjohnwu.magisk.core.download.Action.APK.Upgrade
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.tasks.PatchAPK
import com.topjohnwu.magisk.ktx.relaunchApp
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.superuser.Shell
import java.io.File

private fun Context.patch(apk: File) {
    val patched = File(apk.parent, "patched.apk")
    PatchAPK.patch(this, apk.path, patched.path, packageName, applicationInfo.nonLocalizedLabel)
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

private suspend fun BaseDownloader.upgrade(subject: Subject.Manager) {
    val apk = subject.file.toFile()
    val id = subject.notifyID()
    if (isRunningAsStub) {
        // Move to upgrade location
        apk.copyTo(DynAPK.update(this), overwrite = true)
        apk.delete()
        if (Info.stubChk.version < subject.stub.versionCode) {
            notifyHide(id)
            // Also upgrade stub
            service.fetchFile(subject.stub.link).byteStream().use { it.writeTo(apk) }
            patch(apk)
        } else {
            // Simply relaunch the app
            stopSelf()
            relaunchApp(this)
        }
    } else if (packageName != BuildConfig.APPLICATION_ID) {
        notifyHide(id)
        patch(apk)
    }
}

private fun BaseDownloader.restore(apk: File, id: Int) {
    update(id) {
        it.setProgress(0, 0, true)
            .setProgress(0, 0, true)
            .setContentTitle(getString(R.string.restore_img_msg))
            .setContentText("")
    }
    Config.export()
    Shell.su("pm install $apk && pm uninstall $packageName").exec()
}

suspend fun BaseDownloader.handleAPK(subject: Subject.Manager) =
    when (subject.action) {
        is Upgrade -> upgrade(subject)
        is Restore -> restore(subject.file.toFile(), subject.notifyID())
    }
