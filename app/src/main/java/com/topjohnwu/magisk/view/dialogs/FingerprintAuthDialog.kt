package com.topjohnwu.magisk.view.dialogs

import android.annotation.TargetApi
import android.app.Activity
import android.graphics.Color
import android.hardware.fingerprint.FingerprintManager
import android.os.Build
import android.view.Gravity
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.FingerprintHelper
import com.topjohnwu.magisk.utils.Utils

@TargetApi(Build.VERSION_CODES.M)
class FingerprintAuthDialog(activity: Activity, private val callback: () -> Unit)
    : CustomAlertDialog(activity) {

    private var failureCallback: (() -> Unit)? = null
    private var helper: DialogFingerprintHelper? = null

    init {
        val fingerprint = ContextCompat.getDrawable(activity, R.drawable.ic_fingerprint)
        fingerprint?.setBounds(0, 0, Utils.dpInPx(50), Utils.dpInPx(50))
        val theme = activity.theme
        val ta = theme.obtainStyledAttributes(intArrayOf(R.attr.imageColorTint))
        fingerprint?.setTint(ta.getColor(0, Color.GRAY))
        ta.recycle()
        binding.message.setCompoundDrawables(null, null, null, fingerprint)
        binding.message.compoundDrawablePadding = Utils.dpInPx(20)
        binding.message.gravity = Gravity.CENTER
        setMessage(R.string.auth_fingerprint)
        setNegativeButton(R.string.close) { _, _ ->
            helper?.cancel()
            failureCallback?.invoke()
        }
        setOnCancelListener {
            helper?.cancel()
            failureCallback?.invoke()
        }
        runCatching {
            helper = DialogFingerprintHelper()
        }

    }

    constructor(activity: Activity, onSuccess: () -> Unit, onFailure: () -> Unit)
            : this(activity, onSuccess) {
        failureCallback = onFailure
    }

    override fun show(): AlertDialog {
        return create().apply {
            if (helper == null) {
                dismiss()
                Utils.toast(R.string.auth_fail, Toast.LENGTH_SHORT)
            } else {
                helper?.authenticate()
                show()
            }
        }
    }

    internal inner class DialogFingerprintHelper @Throws(Exception::class)
    constructor() : FingerprintHelper() {

        override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
            binding.message.setTextColor(Color.RED)
            binding.message.text = errString
        }

        override fun onAuthenticationHelp(helpCode: Int, helpString: CharSequence) {
            binding.message.setTextColor(Color.RED)
            binding.message.text = helpString
        }

        override fun onAuthenticationFailed() {
            binding.message.setTextColor(Color.RED)
            binding.message.setText(R.string.auth_fail)
        }

        override fun onAuthenticationSucceeded(result: FingerprintManager.AuthenticationResult) {
            dismiss()
            callback()
        }
    }
}
