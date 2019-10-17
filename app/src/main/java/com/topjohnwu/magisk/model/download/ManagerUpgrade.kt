package com.topjohnwu.magisk.model.download

import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.cmp
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Restore
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Upgrade
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.utils.PatchAPK
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import java.io.File

private fun RemoteFileService.patchPackage(apk: File, id: Int) {
    if (packageName != BuildConfig.APPLICATION_ID) {
        update(id) { notification ->
            notification.setProgress(0, 0, true)
                    .setProgress(0, 0, true)
                    .setContentTitle(getString(R.string.hide_manager_title))
                    .setContentText("")
        }
        val patched = File(apk.parent, "patched.apk")
        PatchAPK.patch(apk, patched, packageName, applicationInfo.nonLocalizedLabel.toString())
        apk.delete()
        patched.renameTo(apk)
    }
}

private fun RemoteFileService.restore(apk: File, id: Int) {
    update(id) { notification ->
        notification.setProgress(0, 0, true)
                .setProgress(0, 0, true)
                .setContentTitle(getString(R.string.restore_img_msg))
                .setContentText("")
    }
    Config.export()
    // Make it world readable
    apk.setReadable(true, false)
    if (Shell.su("pm install $apk").exec().isSuccess) {
        Utils.rmAndLaunch(packageName, SplashActivity::class.java.cmp())
    }
}

fun RemoteFileService.handleAPK(subject: DownloadSubject.Manager)
    = when (subject.configuration) {
        is Upgrade -> patchPackage(subject.file, subject.hashCode())
        is Restore -> restore(subject.file, subject.hashCode())
    }
