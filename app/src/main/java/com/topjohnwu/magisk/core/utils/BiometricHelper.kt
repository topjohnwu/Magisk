package com.topjohnwu.magisk.core.utils

import android.content.Context
import androidx.biometric.BiometricManager
import androidx.biometric.BiometricManager.Authenticators
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config

class BiometricHelper(context: Context) {

    private val mgr = BiometricManager.from(context)

    val isSupported get() = when (mgr.canAuthenticate(Authenticators.BIOMETRIC_WEAK)) {
        BiometricManager.BIOMETRIC_SUCCESS -> true
        else -> false
    }

    private val appBiometric get() = when (Config.suBiometric) {
        Config.Value.BIOMETRIC_MAGISK -> true
        else -> false
    }

    val isEnabled get() = isSupported && appBiometric

    fun authenticate(
        activity: FragmentActivity,
        onError: () -> Unit = {},
        onSuccess: () -> Unit): BiometricPrompt {
        val prompt = BiometricPrompt(activity,
            ContextCompat.getMainExecutor(activity),
            object : BiometricPrompt.AuthenticationCallback() {
                override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                    onError()
                }

                override fun onAuthenticationFailed() {
                    onError()
                }

                override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                    onSuccess()
                }
            }
        )
        val info = BiometricPrompt.PromptInfo.Builder()
            .setConfirmationRequired(true)
            .setAllowedAuthenticators(Authenticators.BIOMETRIC_WEAK)
            .setTitle(activity.getString(R.string.authenticate))
            .setNegativeButtonText(activity.getString(android.R.string.cancel))
            .build()
        prompt.authenticate(info)
        return prompt
    }

}
