package com.topjohnwu.magisk.model.events.dialog

import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell

class UninstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        dialog.applyTitle(R.string.uninstall_magisk_title)
            .applyMessage(R.string.uninstall_magisk_msg)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.restore_img
                preventDismiss = true
                onClick { restore(dialog) }
            }
        if (Info.remote.uninstaller.link.isNotEmpty()) {
            dialog.applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.complete_uninstall
                onClick { completeUninstall() }
            }
        }
    }

    private fun restore(dialog: MagiskDialog) {
        dialog.applyTitle(R.string.restore_img)
            .applyMessage(R.string.restore_img_msg)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                title = ""
            }
            .cancellable(false)

        Shell.su("restore_imgs").submit { result ->
            dialog.dismiss()
            if (result.isSuccess) {
                Utils.toast(R.string.restore_done, Toast.LENGTH_SHORT)
            } else {
                Utils.toast(R.string.restore_fail, Toast.LENGTH_LONG)
            }
        }
    }

    private fun completeUninstall() {
        DownloadService(dialog.context) {
            subject = DownloadSubject.Magisk(Configuration.Uninstall)
        }
    }

}
