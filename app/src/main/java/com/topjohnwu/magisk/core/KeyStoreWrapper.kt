package com.topjohnwu.magisk.core

import android.content.Context
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import java.security.KeyStore
import java.security.SecureRandom
import javax.crypto.KeyGenerator
import javax.crypto.Mac
import javax.crypto.SecretKey
import javax.crypto.spec.SecretKeySpec

/**
 * KeyStoreWrapper
 *
 * - Creates or retrieves an HMAC-SHA256 key stored in AndroidKeyStore.
 * - Provides helpers to compute HMAC over bytes or hex strings.
 *
 * Notes:
 * - Keys generated in AndroidKeyStore are non-exportable. For verification in other environments,
 *   you must use a shared verification key or an exported test key only for CI (see manifest_verify tool).
 * - Requires API level >= 23 for HMAC key generation in AndroidKeyStore.
 */
object KeyStoreWrapper {
    private const val ANDROID_KEYSTORE = "AndroidKeyStore"

    /**
     * Create or get a SecretKey stored in AndroidKeyStore for HMAC-SHA256.
     * alias: alias for the key in KeyStore.
     */
    @JvmStatic
    fun getOrCreateKey(context: Context, alias: String = "rafaelia_hmac_key"): SecretKey {
        try {
            val ks = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
            val existing = ks.getEntry(alias, null)
            if (existing != null) {
                val keyEntry = ks.getKey(alias, null) as? SecretKey
                if (keyEntry != null) return keyEntry
            }
        } catch (e: Exception) {
            // continue to generate
        }

        val kg = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256, ANDROID_KEYSTORE)
        val spec = KeyGenParameterSpec.Builder(
            alias,
            KeyProperties.PURPOSE_SIGN or KeyProperties.PURPOSE_VERIFY
        )
            .setDigests(KeyProperties.DIGEST_SHA256)
            .setUserAuthenticationRequired(false)
            .build()
        kg.init(spec)
        return kg.generateKey()
    }

    /**
     * Compute HMAC-SHA256 raw bytes.
     */
    @JvmStatic
    fun hmacSha256(key: SecretKey, data: ByteArray): ByteArray {
        val mac = Mac.getInstance("HmacSHA256")
        mac.init(key)
        return mac.doFinal(data)
    }

    /**
     * Convenience: HMAC of hex string interpreted as bytes, returns Base64 (NO_WRAP).
     */
    @JvmStatic
    fun hmacSha256HexBase64(key: SecretKey, hexData: String): String {
        val bytes = hexStringToByteArray(hexData)
        val h = hmacSha256(key, bytes)
        return Base64.encodeToString(h, Base64.NO_WRAP)
    }

    /**
     * Export a test key (insecure) for CI/local use only.
     * Returns Base64 encoded raw key bytes. Use only for test pipelines.
     */
    @JvmStatic
    fun createExportableTestKeyForCI(): String {
        val seed = ByteArray(32)
        SecureRandom().nextBytes(seed)
        return Base64.encodeToString(seed, Base64.NO_WRAP)
    }

    /**
     * Build a SecretKey from raw base64 (used only for test/CI where key material must be available).
     * DO NOT use in production with real keys.
     */
    @JvmStatic
    fun secretKeyFromRaw(base64: String): SecretKey {
        val raw = android.util.Base64.decode(base64, Base64.NO_WRAP)
        return SecretKeySpec(raw, "HmacSHA256")
    }

    private fun hexStringToByteArray(s: String): ByteArray {
        val len = s.length
        val data = ByteArray(len / 2)
        var i = 0
        while (i < len) {
            data[i / 2] =
                ((Character.digit(s[i], 16) shl 4) + Character.digit(s[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }
}
