package com.topjohnwu.magisk.core.utils

import androidx.biometric.BiometricManager
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.di.AppContext

object BiometricHelper {

    private val mgr by lazy { BiometricManager.from(AppContext) }

    val isSupported get() = when (mgr.canAuthenticate()) {
        BiometricManager.BIOMETRIC_SUCCESS -> true
        else -> false
    }

    val isEnabled: Boolean get() {
        val enabled = Config.suBiometric
        if (enabled && !isSupported) {
            Config.suBiometric = false
            return false
        }
        return enabled
    }

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
            .setDeviceCredentialAllowed(false)
            .setTitle(activity.getString(R.string.authenticate))
            .setNegativeButtonText(activity.getString(android.R.string.cancel))
            .build()
        prompt.authenticate(info)
        return prompt
    }

}
