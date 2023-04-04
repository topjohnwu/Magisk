package com.topjohnwu.magisk.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.view.MagiskDialog

class SecondSlotWarningDialog : DialogBuilder {

    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(android.R.string.dialog_alert_title)
            setMessage(R.string.install_inactive_slot_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
            }
            setCancelable(true)
        }
    }
}
