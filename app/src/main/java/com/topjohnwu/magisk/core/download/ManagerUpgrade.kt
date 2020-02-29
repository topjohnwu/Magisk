package com.topjohnwu.magisk.core.download

import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.ProcessPhoenix
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.PatchAPK
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Restore
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Upgrade
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.superuser.Shell
import java.io.File

private fun RemoteFileService.patch(apk: File, id: Int) {
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

private fun RemoteFileService.upgrade(apk: File, id: Int) {
    if (isRunningAsStub) {
        // Move to upgrade location
        apk.copyTo(DynAPK.update(this), overwrite = true)
        apk.delete()
        if (Info.stub!!.version < Info.remote.stub.versionCode) {
            // We also want to upgrade stub
            service.fetchFile(Info.remote.stub.link).blockingGet().byteStream().use {
                it.writeTo(apk)
            }
            patch(apk, id)
        } else {
            // Simply relaunch the app
            ProcessPhoenix.triggerRebirth(this, intent<ProcessPhoenix>())
        }
    } else {
        patch(apk, id)
    }
}

private fun RemoteFileService.restore(apk: File, id: Int) {
    update(id) {
        it.setProgress(0, 0, true)
            .setProgress(0, 0, true)
            .setContentTitle(getString(R.string.restore_img_msg))
            .setContentText("")
    }
    Config.export()
    // Make it world readable
    apk.setReadable(true, false)
    Shell.su("pm install $apk && pm uninstall $packageName").exec()
}

fun RemoteFileService.handleAPK(subject: DownloadSubject.Manager) =
    when (subject.configuration) {
        is Upgrade -> upgrade(subject.file, subject.hashCode())
        is Restore -> restore(subject.file, subject.hashCode())
    }
