package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import android.app.ProgressDialog
import android.widget.Toast
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell

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
            setPositiveButton(R.string.complete_uninstall) { _, _ ->
                DownloadService(activity) {
                    subject = DownloadSubject.Magisk(Configuration.Uninstall)
                }
            }
        }
    }
}
