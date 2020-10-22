package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.view.MagiskDialog

class SecondSlotWarningDialog : DialogEvent() {

    override fun build(dialog: MagiskDialog) {
        dialog.applyTitle(android.R.string.dialog_alert_title)
            .applyMessage(R.string.install_inactive_slot_msg)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = android.R.string.ok
            }
            .cancellable(true)
    }
}
