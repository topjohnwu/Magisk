package com.topjohnwu.magisk.model.events.dialog

import android.content.DialogInterface
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.internal.UiThreadHandler
import org.koin.core.KoinComponent

class EnvFixDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) = dialog
        .applyTitle(R.string.env_fix_title)
        .applyMessage(R.string.env_fix_msg)
        .applyButton(MagiskDialog.ButtonType.POSITIVE) {
            titleRes = R.string.yes
            preventDismiss = true
            onClick {
                dialog.applyTitle(R.string.setup_title)
                    .applyMessage(R.string.setup_msg)
                    .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                        title = ""
                    }
                    .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                        title = ""
                    }
                    .cancellable(false)
                fixEnv(it)
            }
        }
        .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
            titleRes = android.R.string.no
        }
        .let { Unit }

    private fun fixEnv(dialog: DialogInterface) {
        object : MagiskInstaller(), KoinComponent {
            override fun operations() = fixEnv()

            override fun onResult(success: Boolean) {
                dialog.dismiss()
                Utils.toast(
                    if (success) R.string.reboot_delay_toast else R.string.setup_fail,
                    Toast.LENGTH_LONG
                )
                if (success)
                    UiThreadHandler.handler.postDelayed({ reboot() }, 5000)
            }
        }.exec()
    }

}
