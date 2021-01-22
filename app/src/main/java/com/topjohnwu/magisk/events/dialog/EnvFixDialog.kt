package com.topjohnwu.magisk.events.dialog

import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.view.MagiskDialog
import kotlinx.coroutines.launch

class EnvFixDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) = dialog
        .applyTitle(R.string.env_fix_title)
        .applyMessage(R.string.env_fix_msg)
        .applyButton(MagiskDialog.ButtonType.POSITIVE) {
            titleRes = android.R.string.ok
            preventDismiss = true
            onClick {
                dialog.applyTitle(R.string.setup_title)
                    .applyMessage(R.string.setup_msg)
                    .resetButtons()
                    .cancellable(false)
                (dialog.ownerActivity as BaseActivity).lifecycleScope.launch {
                    MagiskInstaller.FixEnv {
                        dialog.dismiss()
                    }.exec()
                }
            }
        }
        .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
            titleRes = android.R.string.cancel
        }
        .let { }

    companion object {
        const val DISMISS = "com.topjohnwu.magisk.ENV_DONE"
    }
}
