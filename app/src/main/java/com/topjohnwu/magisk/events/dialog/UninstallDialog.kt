package com.topjohnwu.magisk.events.dialog

import android.app.ProgressDialog
import android.content.Context
import android.util.Log
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell

class UninstallDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.uninstall_magisk_title)
            setMessage(R.string.uninstall_magisk_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = "Yes"
                onClick { completeUninstall(dialog) }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = "No"
                dialog.dismiss()
            }
        }
    }

    private fun completeUninstall(dialog: MagiskDialog) {
        (dialog.ownerActivity as NavigationActivity<*>)
            .navigation.navigate(FlashFragment.uninstall())
    }

}
