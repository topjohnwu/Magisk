package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import android.app.ProgressDialog
import android.content.Intent
import android.net.Uri
import android.widget.Toast
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.ProgressNotification
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import java.io.File

class UninstallDialog(activity: Activity) : CustomAlertDialog(activity) {

    init {
        setTitle(R.string.uninstall_magisk_title)
        setMessage(R.string.uninstall_magisk_msg)
        setNeutralButton(R.string.restore_img) { _, _ ->
            val dialog = ProgressDialog.show(activity,
                    activity.getString(R.string.restore_img),
                    activity.getString(R.string.restore_img_msg))
            Shell.su("restore_imgs").submit { result ->
                dialog.cancel()
                if (result.isSuccess) {
                    Utils.toast(R.string.restore_done, Toast.LENGTH_SHORT)
                } else {
                    Utils.toast(R.string.restore_fail, Toast.LENGTH_LONG)
                }
            }
        }
        if (Info.remote.uninstaller.link.isNotEmpty()) {
            setPositiveButton(R.string.complete_uninstall) { d, i ->
                val zip = File(activity.filesDir, "uninstaller.zip")
                val progress = ProgressNotification(zip.name)
                Networking.get(Info.remote.uninstaller.link)
                        .setDownloadProgressListener(progress)
                        .setErrorHandler { _, _ -> progress.dlFail() }
                        .getAsFile(zip) { f ->
                            progress.dismiss()
                            val intent = Intent(activity, ClassMap[FlashActivity::class.java])
                                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    .setData(Uri.fromFile(f))
                                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL)
                            activity.startActivity(intent)
                        }
            }
        }
    }
}
