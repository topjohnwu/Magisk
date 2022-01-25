package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.tasks.HideAPK
import com.topjohnwu.magisk.view.MagiskDialog

class RestoreAppDialog : DialogEvent() {
    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.settings_restore_app_title)
            setMessage(R.string.restore_app_confirmation)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                onClick { HideAPK.restore(dialog.ownerActivity!!) }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = android.R.string.cancel
            }
            setCancelable(true)
        }
    }
}
