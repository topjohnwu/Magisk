package com.topjohnwu.magisk.model.download

import android.content.ComponentName
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Restore
import com.topjohnwu.magisk.model.entity.internal.Configuration.APK.Upgrade
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.utils.DynamicClassLoader
import com.topjohnwu.magisk.utils.PatchAPK
import com.topjohnwu.magisk.utils.RootUtils
import com.topjohnwu.superuser.Shell
import timber.log.Timber
import java.io.File

private fun RemoteFileService.patchPackage(apk: File, id: Int): File {
    if (packageName != BuildConfig.APPLICATION_ID) {
        update(id) { notification ->
            notification.setProgress(0, 0, true)
                    .setProgress(0, 0, true)
                    .setContentTitle(getString(R.string.hide_manager_title))
                    .setContentText("")
        }
        val patched = File(apk.parent, "patched.apk")
        try {
            // Try using the new APK to patch itself
            val loader = DynamicClassLoader(apk)
            loader.loadClass("a.a")
                    .getMethod("patchAPK", String::class.java, String::class.java, String::class.java)
                    .invoke(null, apk.path, patched.path, packageName)
        } catch (e: Exception) {
            Timber.e(e)
            // Fallback to use the current implementation
            PatchAPK.patch(apk.path, patched.path, packageName)
        }
        apk.delete()
        return patched
    } else {
        return apk
    }
}

private fun RemoteFileService.restore(apk: File, id: Int): File {
    update(id) { notification ->
        notification.setProgress(0, 0, true)
                .setProgress(0, 0, true)
                .setContentTitle(getString(R.string.restore_img_msg))
                .setContentText("")
    }
    Config.export()
    // Make it world readable
    apk.setReadable(true, false)
    if (Shell.su("pm install $apk").exec().isSuccess)
        RootUtils.rmAndLaunch(packageName,
                ComponentName(BuildConfig.APPLICATION_ID,
                        ClassMap.get<Class<*>>(SplashActivity::class.java).name))
    return apk
}

fun RemoteFileService.handleAPK(apk: File, subject: DownloadSubject.Manager)
    = when (subject.configuration) {
        is Upgrade -> patchPackage(apk, subject.hashCode())
        is Restore -> restore(apk, subject.hashCode())
    }
