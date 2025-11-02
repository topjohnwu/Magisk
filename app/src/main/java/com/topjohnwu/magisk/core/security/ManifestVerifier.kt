package com.topjohnwu.magisk.core.security

import android.util.Base64
import org.json.JSONObject
import java.io.File
import java.nio.charset.StandardCharsets
import java.security.MessageDigest
import javax.crypto.Mac
import javax.crypto.SecretKey

object ManifestVerifier {
    /**
     * Verify that given manifest file (JSON) matches backup file content SHA and HMAC.
     * - manifestFile: JSON with fields backupPath, sha256, hmac_b64
     * - keyProvider: lambda returning SecretKey for HMAC-SHA256 verification (nullable)
     *
     * Returns true if verification passes (sha and hmac match).
     */
    fun verifyManifest(manifestFile: File, keyProvider: () -> SecretKey?): Boolean {
        if (!manifestFile.exists()) return false
        val json = JSONObject(manifestFile.readText(StandardCharsets.UTF_8))
        val backupPath = json.optString("backupPath", "")
        val expectedSha = json.optString("sha256", "")
        val expectedHmacB64 = json.optString("hmac_b64", "")

        if (backupPath.isEmpty() || expectedSha.isEmpty()) return false
        val backup = File(backupPath)
        if (!backup.exists()) return false

        val digest = MessageDigest.getInstance("SHA-256")
        backup.inputStream().use { input ->
            val buf = ByteArray(16 * 1024)
            var r: Int
            while (input.read(buf).also { r = it } != -1) digest.update(buf, 0, r)
        }
        val actualSha = digest.digest().joinToString("") { String.format("%02x", it) }
        if (!actualSha.equals(expectedSha, ignoreCase = true)) return false

        if (expectedHmacB64.isNullOrEmpty()) return true // only sha required

        val key = try { keyProvider() } catch (e: Exception) { null }
        val mac = Mac.getInstance("HmacSHA256")
        val sk = key ?: return false
        mac.init(sk)
        val computed = mac.doFinal(hexStringToByteArray(actualSha))
        val computedB64 = Base64.encodeToString(computed, Base64.NO_WRAP)
        return computedB64 == expectedHmacB64
    }

    private fun hexStringToByteArray(s: String): ByteArray {
        val len = s.length
        val data = ByteArray(len / 2)
        var i = 0
        while (i < len) {
            data[i / 2] = ((Character.digit(s[i], 16) shl 4) + Character.digit(s[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }
}
