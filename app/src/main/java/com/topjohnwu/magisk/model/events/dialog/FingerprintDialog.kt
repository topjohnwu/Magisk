package com.topjohnwu.magisk.model.events.dialog

import android.hardware.fingerprint.FingerprintManager
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.FingerprintHelper
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog

class FingerprintDialog(
    builder: Builder.() -> Unit
) : DialogEvent() {

    private val callbacks = Builder().apply(builder)
    private var helper: Helper? = null
        get() {
            if (field == null) {
                runCatching { field = Helper() }
            }
            return field
        }

    override fun build(dialog: MagiskDialog) {
        dialog.applyIcon(R.drawable.ic_fingerprint)
            .applyTitle(R.string.auth_fingerprint)
            .cancellable(false) //possible fix for devices that have flawed under-screen sensor implementation
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = android.R.string.cancel
                onClick {
                    callbacks.listenerOnFailure()
                    helper?.cancel()
                }
            }
            .onShow {
                helper?.authenticate() ?: it.let {
                    callbacks.listenerOnFailure()
                    Utils.toast(R.string.auth_fail, Toast.LENGTH_SHORT)
                    it.dismiss()
                }
            }
    }

    inner class Builder internal constructor() {
        internal var listenerOnFailure: GenericDialogListener = {}
        internal var listenerOnSuccess: GenericDialogListener = {}

        fun onFailure(listener: GenericDialogListener) {
            listenerOnFailure = listener
        }

        fun onSuccess(listener: GenericDialogListener) {
            listenerOnSuccess = listener
        }
    }

    private inner class Helper @Throws(Exception::class) constructor() : FingerprintHelper() {
        override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
            dialog.applyMessage(errString)
        }

        override fun onAuthenticationHelp(helpCode: Int, helpString: CharSequence) {
            dialog.applyMessage(helpString)
        }

        override fun onAuthenticationFailed() {
            dialog.applyMessage(R.string.auth_fail)
        }

        override fun onAuthenticationSucceeded(result: FingerprintManager.AuthenticationResult) {
            callbacks.listenerOnSuccess()
            dialog.dismiss()
        }
    }
}