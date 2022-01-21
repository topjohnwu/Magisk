package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.view.MagiskDialog

class SuperuserRevokeDialog(
    builder: Builder.() -> Unit
) : DialogEvent() {

    private val callbacks = Builder().apply(builder)

    override fun build(dialog: MagiskDialog) {
        dialog.apply {
            setTitle(R.string.su_revoke_title)
            setMessage(R.string.su_revoke_msg, callbacks.appName)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                onClick { callbacks.listenerOnSuccess() }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = android.R.string.cancel
            }
        }
    }

    inner class Builder internal constructor() {
        var appName: String = ""

        internal var listenerOnSuccess: GenericDialogListener = {}

        fun onSuccess(listener: GenericDialogListener) {
            listenerOnSuccess = listener
        }
    }
}
