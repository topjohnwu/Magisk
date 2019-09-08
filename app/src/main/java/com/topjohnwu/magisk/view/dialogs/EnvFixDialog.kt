package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import android.app.ProgressDialog
import android.widget.Toast
import androidx.core.net.toUri
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.cachedFile
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.tasks.MagiskInstaller
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.net.Networking
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.io.SuFile
import java.io.File

class EnvFixDialog(activity: Activity) : CustomAlertDialog(activity) {

    init {
        setTitle(R.string.env_fix_title)
        setMessage(R.string.env_fix_msg)
        setCancelable(true)
        setPositiveButton(R.string.yes) { _, _ ->
            val pd = ProgressDialog.show(activity,
                    activity.getString(R.string.setup_title),
                    activity.getString(R.string.setup_msg))
            object : MagiskInstaller() {
                override fun operations(): Boolean {
                    installDir = SuFile("/data/adb/magisk")
                    Shell.su("rm -rf /data/adb/magisk/*").exec()
                    val zip : File = activity.cachedFile("magisk.zip")
                    if (!ShellUtils.checkSum("MD5", zip, Info.remote.magisk.md5))
                        Networking.get(Info.remote.magisk.link).execForFile(zip)
                    zipUri = zip.toUri()
                    return extractZip() && Shell.su("fix_env").exec().isSuccess
                }

                override fun onResult(success: Boolean) {
                    pd.dismiss()
                    Utils.toast(if (success) R.string.reboot_delay_toast else R.string.setup_fail, Toast.LENGTH_LONG)
                    if (success)
                        UiThreadHandler.handler.postDelayed({ reboot() }, 5000)
                }
            }.exec()
        }
        setNegativeButton(R.string.no_thanks, null)
    }
}
