package com.topjohnwu.magisk.dialog

import android.app.ProgressDialog
import android.content.Context
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell

class UninstallDialog : DialogBuilder {

    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.uninstall_magisk_title)
            setMessage(R.string.uninstall_magisk_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = R.string.restore_img
                onClick { restore(dialog.context) }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = R.string.complete_uninstall
                onClick { completeUninstall(dialog) }
            }
        }
    }

    @Suppress("DEPRECATION")
    private fun restore(context: Context) {
        val dialog = ProgressDialog(context).apply {
            setMessage(context.getString(R.string.restore_img_msg))
            show()
        }

        Shell.cmd("restore_imgs").submit { result ->
            dialog.dismiss()
            if (result.isSuccess) {
                context.toast(R.string.restore_done, Toast.LENGTH_SHORT)
            } else {
                context.toast(R.string.restore_fail, Toast.LENGTH_LONG)
            }
        }
    }

    private fun completeUninstall(dialog: MagiskDialog) {
        (dialog.ownerActivity as NavigationActivity<*>)
            .navigation.navigate(FlashFragment.uninstall())
    }

}
