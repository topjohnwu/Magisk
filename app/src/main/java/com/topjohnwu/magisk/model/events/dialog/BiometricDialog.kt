package com.topjohnwu.magisk.model.events.dialog

import android.os.Handler
import androidx.appcompat.app.AppCompatActivity
import androidx.biometric.BiometricPrompt
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ViewEvent

class BiometricDialog(
    builder: Builder.() -> Unit
) : ViewEvent(), ActivityExecutor {

    private var listenerOnFailure: GenericDialogListener = {}
    private var listenerOnSuccess: GenericDialogListener = {}

    init {
        builder(Builder())
    }

    override fun invoke(activity: AppCompatActivity) {
        val handler = Handler()
        val prompt = BiometricPrompt.PromptInfo.Builder()
            .setNegativeButtonText(activity.getString(android.R.string.cancel))
            .build()

        val callback = object : BiometricPrompt.AuthenticationCallback() {
            override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                listenerOnFailure()
            }

            override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                listenerOnSuccess()
            }

            override fun onAuthenticationFailed() {
                listenerOnFailure()
            }
        }
        BiometricPrompt(activity, { handler.post(it) }, callback)
            .authenticate(prompt/*launch with no crypto for now*/)
    }

    inner class Builder internal constructor() {

        fun onFailure(listener: GenericDialogListener) {
            listenerOnFailure = listener
        }

        fun onSuccess(listener: GenericDialogListener) {
            listenerOnSuccess = listener
        }
    }

}