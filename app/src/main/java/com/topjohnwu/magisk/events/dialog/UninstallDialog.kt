package com.topjohnwu.magisk.events.dialog

import android.app.ProgressDialog
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell

class UninstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        dialog.applyTitle(R.string.uninstall_magisk_title)
            .applyMessage(R.string.uninstall_magisk_msg)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.restore_img
                onClick { restore() }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.complete_uninstall
                onClick { completeUninstall() }
            }
    }

    @Suppress("DEPRECATION")
    private fun restore() {
        val dialog = ProgressDialog(dialog.context).apply {
            setMessage(dialog.context.getString(R.string.restore_img_msg))
            show()
        }

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
        (dialog.ownerActivity as? BaseUIActivity<*, *>)
                ?.navigation?.navigate(FlashFragment.uninstall())
    }

}
