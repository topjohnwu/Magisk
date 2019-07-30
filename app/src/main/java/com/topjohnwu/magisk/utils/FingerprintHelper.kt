package com.topjohnwu.magisk.utils

import android.annotation.TargetApi
import android.app.KeyguardManager
import android.content.Context
import android.hardware.fingerprint.FingerprintManager
import android.os.Build
import android.os.CancellationSignal
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.inject
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey

@TargetApi(Build.VERSION_CODES.M)
abstract class FingerprintHelper @Throws(Exception::class)
protected constructor() {

    private val manager: FingerprintManager?
    private val cipher: Cipher
    private var cancel: CancellationSignal? = null
    private val context: Context by inject()

    init {
        val keyStore = KeyStore.getInstance("AndroidKeyStore")
        manager = context.getSystemService(FingerprintManager::class.java)
        cipher = Cipher.getInstance(KeyProperties.KEY_ALGORITHM_AES + "/"
                + KeyProperties.BLOCK_MODE_CBC + "/"
                + KeyProperties.ENCRYPTION_PADDING_PKCS7)
        keyStore.load(null)
        var key = keyStore.getKey(SU_KEYSTORE_KEY, null) as SecretKey? ?: generateKey()
        runCatching {
            cipher.init(Cipher.ENCRYPT_MODE, key)
        }.onFailure {
            // Only happens on Marshmallow
            key = generateKey()
            cipher.init(Cipher.ENCRYPT_MODE, key)
        }
    }

    abstract fun onAuthenticationError(errorCode: Int, errString: CharSequence)

    abstract fun onAuthenticationHelp(helpCode: Int, helpString: CharSequence)

    abstract fun onAuthenticationSucceeded(result: FingerprintManager.AuthenticationResult)

    abstract fun onAuthenticationFailed()

    fun authenticate() {
        cancel = CancellationSignal()
        val cryptoObject = FingerprintManager.CryptoObject(cipher)
        manager!!.authenticate(cryptoObject, cancel, 0, Callback(), null)
    }

    fun cancel() {
        if (cancel != null)
            cancel!!.cancel()
    }

    @Throws(Exception::class)
    private fun generateKey(): SecretKey {
        val keygen = KeyGenerator
                .getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore")
        val builder = KeyGenParameterSpec.Builder(
                SU_KEYSTORE_KEY,
                KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_CBC)
                .setUserAuthenticationRequired(true)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_PKCS7)

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            builder.setInvalidatedByBiometricEnrollment(false)
        }
        keygen.init(builder.build())
        return keygen.generateKey()
    }

    private inner class Callback : FingerprintManager.AuthenticationCallback() {
        override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
            this@FingerprintHelper.onAuthenticationError(errorCode, errString)
        }

        override fun onAuthenticationHelp(helpCode: Int, helpString: CharSequence) {
            this@FingerprintHelper.onAuthenticationHelp(helpCode, helpString)
        }

        override fun onAuthenticationSucceeded(result: FingerprintManager.AuthenticationResult) {
            this@FingerprintHelper.onAuthenticationSucceeded(result)
        }

        override fun onAuthenticationFailed() {
            this@FingerprintHelper.onAuthenticationFailed()
        }
    }

    companion object {
        private const val SU_KEYSTORE_KEY = "su_key"

        fun useFingerprint(): Boolean {
            var fp = Config.suFingerprint
            if (fp && !canUseFingerprint()) {
                Config.suFingerprint = false
                fp = false
            }
            return fp
        }

        fun canUseFingerprint(context: Context = get()): Boolean {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M)
                return false
            val km = context.getSystemService(KeyguardManager::class.java)
            val fm = context.getSystemService(FingerprintManager::class.java)
            return km?.isKeyguardSecure ?: false &&
                    fm != null && fm.isHardwareDetected && fm.hasEnrolledFingerprints()
        }
    }
}
