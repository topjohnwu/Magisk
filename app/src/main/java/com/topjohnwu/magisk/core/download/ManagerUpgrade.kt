package com.topjohnwu.magisk.core.download

import androidx.core.net.toFile
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Action.APK.Restore
import com.topjohnwu.magisk.core.download.Action.APK.Upgrade
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.PatchAPK
import com.topjohnwu.magisk.ktx.relaunchApp
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.utils.APKInstall
import com.topjohnwu.superuser.Shell
import java.io.File

private fun DownloadService.patch(apk: File, id: Int) {
    if (packageName == BuildConfig.APPLICATION_ID)
        return

    update(id) {
        it.setProgress(0, 0, true)
            .setProgress(0, 0, true)
            .setContentTitle(getString(R.string.hide_manager_title))
            .setContentText("")
    }
    val patched = File(apk.parent, "patched.apk")
    PatchAPK.patch(apk.path, patched.path, packageName, applicationInfo.nonLocalizedLabel)
    apk.delete()
    patched.renameTo(apk)
}

private suspend fun DownloadService.upgrade(apk: File, id: Int) {
    if (isRunningAsStub) {
        // Move to upgrade location
        apk.copyTo(DynAPK.update(this), overwrite = true)
        apk.delete()
        if (Info.stubChk.version < Info.remote.stub.versionCode) {
            // We also want to upgrade stub
            service.fetchFile(Info.remote.stub.link).byteStream().use {
                it.writeTo(apk)
            }
            patch(apk, id)
        } else {
            // Simply relaunch the app
            stopSelf()
            relaunchApp(this)
        }
    } else {
        patch(apk, id)
    }
    APKInstall.install(this, apk)
}

private fun DownloadService.restore(apk: File, id: Int) {
    update(id) {
        it.setProgress(0, 0, true)
            .setProgress(0, 0, true)
            .setContentTitle(getString(R.string.restore_img_msg))
            .setContentText("")
    }
    Config.export()
    Shell.su("pm install $apk && pm uninstall $packageName").exec()
}

suspend fun DownloadService.handleAPK(subject: Subject.Manager) =
    when (subject.action) {
        is Upgrade -> upgrade(subject.file.toFile(), subject.notifyID())
        is Restore -> restore(subject.file.toFile(), subject.notifyID())
    }
