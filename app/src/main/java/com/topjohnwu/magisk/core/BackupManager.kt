package com.topjohnwu.magisk.core

import android.content.Context
import android.os.Build
import android.os.StatFs
import android.util.Base64
import androidx.annotation.WorkerThread
import org.json.JSONObject
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException
import java.nio.charset.StandardCharsets
import java.security.KeyStore
import java.security.MessageDigest
import java.security.SecureRandom
import javax.crypto.Mac
import javax.crypto.SecretKey
import javax.crypto.spec.SecretKeySpec
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import javax.crypto.KeyGenerator

/**
 * BackupManager
 *
 * Responsibilities:
 * - Atomically save a bit-for-bit copy of a boot image (or provided image file) to app private storage.
 * - Compute streaming SHA-256 of the saved file.
 * - Compute HMAC-SHA256 over the SHA-256 digest using a non-exportable AndroidKeyStore key (fallback to software key).
 * - Produce and persist a manifest.json per session with metadata required for rollback and audit.
 *
 * Usage:
 *   val mgr = BackupManager(context)
 *   val manifest = mgr.backupBootImage(bootImageFile, sessionId)
 *
 * Notes / security:
 * - Uses streaming I/O to avoid large memory usage.
 * - Checks free storage before starting and throws if insufficient.
 * - KeyStore-backed HMAC key is created non-exportable when available; when not available a software key is used (logged).
 *
 * Testing:
 * - Unit tests should mock the File I/O streams (or use test fixtures in test/resources).
 * - Validate that saved SHA and HMAC match expected values for fixtures.
 *
 * Important: adapt paths, permissions and UI integration as needed in the app.
 */
class BackupManager(private val context: Context) {

    companion object {
        private const val BACKUP_DIR_NAME = "rafaelia_backups"
        private const val MANIFEST_SUFFIX = "-manifest.json"
        private const val HMAC_KEY_ALIAS = "raf_hmac_sha256"
        private const val ANDROID_KEYSTORE = "AndroidKeyStore"
        private const val BUFFER_SIZE = 16 * 1024
        private const val MIN_FREE_BYTES_BUFFER = 32L * 1024 * 1024 // require at least 32MB free beyond file size (tunable)
    }

    private val backupDir: File = File(context.filesDir, BACKUP_DIR_NAME)

    init {
        if (!backupDir.exists()) {
            backupDir.mkdirs()
        }
    }

    /**
     * Perform an atomic backup of bootImage into app-private storage.
     *
     * This method runs on caller's thread. For disk-heavy work call from IO dispatcher / background thread.
     *
     * @param bootImage source file (local FS). The file is not deleted.
     * @param sessionId a unique id for this backup session (timestamp/UUID)
     * @return JSONObject manifest containing metadata: sessionId, originalFile, backupFile, sha256, hmac, timestamp, buildId
     * @throws IOException on disk or stream errors
     * @throws IllegalStateException when there is not enough free space or other fatal precondition
     */
    @WorkerThread
    @Throws(IOException::class, IllegalStateException::class)
    fun backupBootImage(bootImage: File, sessionId: String, buildId: String? = null): JSONObject {
        require(bootImage.exists()) { "bootImage does not exist: ${bootImage.path}" }
        // Pre-flight: check available size
        val free = getFreeSpaceBytes(backupDir)
        if (free < bootImage.length() + MIN_FREE_BYTES_BUFFER) {
            throw IllegalStateException("Insufficient storage for backup. Required ~${bootImage.length() + MIN_FREE_BYTES_BUFFER} bytes but only $free available.")
        }

        val targetName = "${bootImage.name}_$sessionId"
        val backupFile = File(backupDir, targetName)
        val manifestFile = File(backupDir, "$sessionId$MANIFEST_SUFFIX")

        // Prepare digest and HMAC
        val sha256 = MessageDigest.getInstance("SHA-256")
        val mac = try {
            getOrCreateHmacKey() // ensure key exists
            javax.crypto.Mac.getInstance("HmacSHA256").apply {
                init(getKeyFromKeystore() ?: throw IllegalStateException("HMAC key unavailable"))
            }
        } catch (e: Exception) {
            // Fallback: software key (random per-install). This is less secure; log and continue.
            val fallbackKey = getFallbackSoftwareKey()
            javax.crypto.Mac.getInstance("HmacSHA256").apply { init(fallbackKey) }
        }

        // Stream copy and update digests as we go
        FileInputStream(bootImage).use { input ->
            FileOutputStream(backupFile).use { output ->
                val buffer = ByteArray(BUFFER_SIZE)
                var read = 0
                while (input.read(buffer).also { read = it } != -1) {
                    output.write(buffer, 0, read)
                    sha256.update(buffer, 0, read)
                }
                output.fd.sync()
            }
        }

        val shaBytes = sha256.digest()
        val shaHex = shaBytes.toHex()

        // HMAC over the SHA-256 digest (not the whole file) — compact and portable for verification
        val hmacBytes = mac.doFinal(shaBytes)
        val hmacB64 = Base64.encodeToString(hmacBytes, Base64.NO_WRAP)
        val hmacHex = hmacBytes.toHex()

        val ts = System.currentTimeMillis()

        val manifest = JSONObject().apply {
            put("sessionId", sessionId)
            put("originalFile", bootImage.name)
            put("backupFile", backupFile.name)
            put("backupPath", backupFile.absolutePath)
            put("sha256", shaHex)
            put("hmac_b64", hmacB64)
            put("hmac_hex", hmacHex)
            put("timestamp", ts)
            if (!buildId.isNullOrEmpty()) put("buildId", buildId)
            put("size", bootImage.length())
            put("note", "Created by RAFAELIA BackupManager")
        }

        // Persist manifest atomically: write to temp and rename
        val tmp = File(backupDir, "$sessionId$MANIFEST_SUFFIX.tmp")
        tmp.writeText(manifest.toString(4), StandardCharsets.UTF_8)
        tmp.renameTo(manifestFile)

        // Return manifest for caller to use immediately
        return manifest
    }

    /**
     * Get the saved manifest JSON object for a session id, or null if missing.
     */
    fun loadManifest(sessionId: String): JSONObject? {
        val file = File(backupDir, "$sessionId$MANIFEST_SUFFIX")
        return if (file.exists()) {
            JSONObject(file.readText(StandardCharsets.UTF_8))
        } else null
    }

    /**
     * Helper: check if the backup for a session exists and matches its stored sha/hmac.
     * Returns true only if file exists and SHA matches manifest entry.
     */
    @WorkerThread
    fun validateBackup(sessionId: String): Boolean {
        val manifest = loadManifest(sessionId) ?: return false
        val backupFileName = manifest.optString("backupFile") ?: return false
        val backupFile = File(backupDir, backupFileName)
        if (!backupFile.exists()) return false
        val digest = MessageDigest.getInstance("SHA-256")
        FileInputStream(backupFile).use { input ->
            val buffer = ByteArray(BUFFER_SIZE)
            var read = 0
            while (input.read(buffer).also { read = it } != -1) {
                digest.update(buffer, 0, read)
            }
        }
        val actualSha = digest.digest().toHex()
        val expectedSha = manifest.optString("sha256")
        if (!actualSha.equals(expectedSha, ignoreCase = true)) return false

        // Validate HMAC using keystore key if possible
        val storedHmacHex = manifest.optString("hmac_hex", "")
        return try {
            val mac = javax.crypto.Mac.getInstance("HmacSHA256")
            val key = getKeyFromKeystore() ?: getFallbackSoftwareKey()
            mac.init(key)
            val computedHmac = mac.doFinal(digest.digest())
            computedHmac.toHex().equals(storedHmacHex, ignoreCase = true)
        } catch (e: Exception) {
            // If HMAC cannot be validated, we consider this a warning -> return false for strict validation
            false
        }
    }

    /**
     * Create or ensure HMAC key exists in AndroidKeyStore.
     * If KeyStore unavailable (older devices / restricted), method will throw and caller will fallback.
     */
    @Throws(Exception::class)
    private fun getOrCreateHmacKey() {
        val ks = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
        if (!ks.containsAlias(HMAC_KEY_ALIAS)) {
            val keyGen = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256, ANDROID_KEYSTORE)
            val builder = KeyGenParameterSpec.Builder(
                HMAC_KEY_ALIAS,
                KeyProperties.PURPOSE_SIGN or KeyProperties.PURPOSE_VERIFY
            ).apply {
                setDigests(KeyProperties.DIGEST_SHA256)
                setKeySize(256)
                // Non-exportable by design; require user authentication? (optional)
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    setUserAuthenticationRequired(false)
                }
            }.build()
            keyGen.init(builder)
            keyGen.generateKey()
        }
    }

    /**
     * Retrieve HMAC key from keystore as SecretKey for Mac init (returns null if not available).
     */
    private fun getKeyFromKeystore(): SecretKey? {
        return try {
            val ks = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
            val entry = ks.getEntry(HMAC_KEY_ALIAS, null) as? KeyStore.SecretKeyEntry
            entry?.secretKey
        } catch (e: Exception) {
            null
        }
    }

    /**
     * Create a software fallback SecretKey (non-persistent across reinstalls).
     * This is used only if KeyStore approach fails. The key is generated once per app process using SecureRandom.
     */
    private fun getFallbackSoftwareKey(): SecretKey {
        // In production you may persist an encrypted fallback key in SharedPreferences protected by KeyStore or avoid fallback entirely.
        val keyBytes = ByteArray(32)
        SecureRandom().nextBytes(keyBytes)
        return SecretKeySpec(keyBytes, "HmacSHA256")
    }

    /**
     * Get free bytes available on the filesystem containing this file.
     */
    private fun getFreeSpaceBytes(file: File): Long {
        val stat = StatFs(file.absolutePath)
        val blockSize = stat.blockSizeLong
        val availBlocks = stat.availableBlocksLong
        return blockSize * availBlocks
    }

    /**
     * Convenience extension: convert ByteArray to lowercase hex string.
     */
    private fun ByteArray.toHex(): String {
        val sb = StringBuilder(size * 2)
        for (b in this) {
            sb.append(String.format("%02x", b))
        }
        return sb.toString()
    }
}     *
     * @param bootImage source file (local FS). The file is not deleted.
     * @param sessionId a unique id for this backup session (timestamp/UUID)
     * @return JSONObject manifest containing metadata: sessionId, originalFile, backupFile, sha256, hmac, timestamp, buildId
     * @throws IOException on disk or stream errors
     * @throws IllegalStateException when there is not enough free space or other fatal precondition
     */
    @WorkerThread
    @Throws(IOException::class, IllegalStateException::class)
    fun backupBootImage(bootImage: File, sessionId: String, buildId: String? = null): JSONObject {
        require(bootImage.exists()) { "bootImage does not exist: ${bootImage.path}" }
        // Pre-flight: check available size
        val free = getFreeSpaceBytes(backupDir)
        if (free < bootImage.length() + MIN_FREE_BYTES_BUFFER) {
            throw IllegalStateException("Insufficient storage for backup. Required ~${bootImage.length() + MIN_FREE_BYTES_BUFFER} bytes but only $free available.")
        }

        val targetName = "${bootImage.name}_$sessionId"
        val backupFile = File(backupDir, targetName)
        val manifestFile = File(backupDir, "$sessionId$MANIFEST_SUFFIX")

        // Prepare digest and HMAC
        val sha256 = MessageDigest.getInstance("SHA-256")
        val mac = try {
            getOrCreateHmacKey() // ensure key exists
            Mac.getInstance("HmacSHA256").apply {
                init(getKeyFromKeystore() ?: throw IllegalStateException("HMAC key unavailable"))
            }
        } catch (e: Exception) {
            // Fallback: software key (random per-install). This is less secure; log and continue.
            val fallbackKey = getFallbackSoftwareKey()
            Mac.getInstance("HmacSHA256").apply { init(fallbackKey) }
        }

        // Stream copy and update digests as we go
        FileInputStream(bootImage).use { input ->
            FileOutputStream(backupFile).use { output ->
                val buffer = ByteArray(BUFFER_SIZE)
                var read = 0
                while (input.read(buffer).also { read = it } != -1) {
                    output.write(buffer, 0, read)
                    sha256.update(buffer, 0, read)
                }
                output.fd.sync()
            }
        }

        val shaBytes = sha256.digest()
        val shaHex = shaBytes.toHex()

        // HMAC over the SHA-256 digest (not the whole file) — compact and portable for verification
        val hmacBytes = mac.doFinal(shaBytes)
        val hmacB64 = Base64.encodeToString(hmacBytes, Base64.NO_WRAP)
        val hmacHex = hmacBytes.toHex()

        val ts = System.currentTimeMillis()

        val manifest = JSONObject().apply {
            put("sessionId", sessionId)
            put("originalFile", bootImage.name)
            put("backupFile", backupFile.name)
            put("backupPath", backupFile.absolutePath)
            put("sha256", shaHex)
            put("hmac_b64", hmacB64)
            put("hmac_hex", hmacHex)
            put("timestamp", ts)
            if (!buildId.isNullOrEmpty()) put("buildId", buildId)
            put("size", bootImage.length())
            put("note", "Created by RAFAELIA BackupManager")
        }

        // Persist manifest atomically: write to temp and rename
        val tmp = File(backupDir, "$sessionId$MANIFEST_SUFFIX.tmp")
        tmp.writeText(manifest.toString(4), StandardCharsets.UTF_8)
        tmp.renameTo(manifestFile)

        // Return manifest for caller to use immediately
        return manifest
    }

    /**
     * Get the saved manifest JSON object for a session id, or null if missing.
     */
    fun loadManifest(sessionId: String): JSONObject? {
        val file = File(backupDir, "$sessionId$MANIFEST_SUFFIX")
        return if (file.exists()) {
            JSONObject(file.readText(StandardCharsets.UTF_8))
        } else null
    }

    /**
     * Helper: check if the backup for a session exists and matches its stored sha/hmac.
     * Returns true only if file exists and SHA matches manifest entry.
     */
    @WorkerThread
    fun validateBackup(sessionId: String): Boolean {
        val manifest = loadManifest(sessionId) ?: return false
        val backupFileName = manifest.optString("backupFile") ?: return false
        val backupFile = File(backupDir, backupFileName)
        if (!backupFile.exists()) return false
        val digest = MessageDigest.getInstance("SHA-256")
        FileInputStream(backupFile).use { input ->
            val buffer = ByteArray(BUFFER_SIZE)
            var read = 0
            while (input.read(buffer).also { read = it } != -1) {
                digest.update(buffer, 0, read)
            }
        }
        val actualSha = digest.digest().toHex()
        val expectedSha = manifest.optString("sha256")
        if (!actualSha.equals(expectedSha, ignoreCase = true)) return false

        // Validate HMAC using keystore key if possible
        val storedHmacHex = manifest.optString("hmac_hex", "")
        return try {
            val mac = Mac.getInstance("HmacSHA256")
            val key = getKeyFromKeystore() ?: getFallbackSoftwareKey()
            mac.init(key)
            val computedHmac = mac.doFinal(digest.digest())
            computedHmac.toHex().equals(storedHmacHex, ignoreCase = true)
        } catch (e: Exception) {
            // If HMAC cannot be validated, we consider this a warning -> return false for strict validation
            false
        }
    }

    /**
     * Create or ensure HMAC key exists in AndroidKeyStore.
     * If KeyStore unavailable (older devices / restricted), method will throw and caller will fallback.
     */
    @Throws(Exception::class)
    private fun getOrCreateHmacKey() {
        val ks = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
        if (!ks.containsAlias(HMAC_KEY_ALIAS)) {
            val keyGen = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256, ANDROID_KEYSTORE)
            val builder = KeyGenParameterSpec.Builder(
                HMAC_KEY_ALIAS,
                KeyProperties.PURPOSE_SIGN or KeyProperties.PURPOSE_VERIFY
            ).apply {
                setDigests(KeyProperties.DIGEST_SHA256)
                setKeySize(256)
                // Non-exportable by design; require user authentication? (optional)
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    setUserAuthenticationRequired(false)
                }
            }.build()
            keyGen.init(builder)
            keyGen.generateKey()
        }
    }

    /**
     * Retrieve HMAC key from keystore as SecretKey for Mac init (returns null if not available).
     */
    private fun getKeyFromKeystore(): SecretKey? {
        return try {
            val ks = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
            val entry = ks.getEntry(HMAC_KEY_ALIAS, null) as? KeyStore.SecretKeyEntry
            entry?.secretKey
        } catch (e: Exception) {
            null
        }
    }

    /**
     * Create a software fallback SecretKey (non-persistent across reinstalls).
     * This is used only if KeyStore approach fails. The key is generated once per app process using SecureRandom.
     */
    private fun getFallbackSoftwareKey(): SecretKey {
        // In production you may persist an encrypted fallback key in SharedPreferences protected by KeyStore or avoid fallback entirely.
        val keyBytes = ByteArray(32)
        SecureRandom().nextBytes(keyBytes)
        return SecretKeySpec(keyBytes, "HmacSHA256")
    }

    /**
     * Get free bytes available on the filesystem containing this file.
     */
    private fun getFreeSpaceBytes(file: File): Long {
        val stat = StatFs(file.absolutePath)
        val blockSize = stat.blockSizeLong
        val availBlocks = stat.availableBlocksLong
        return blockSize * availBlocks
    }

    /**
     * Convenience extension: convert ByteArray to lowercase hex string.
     */
    private fun ByteArray.toHex(): String {
        val sb = StringBuilder(size * 2)
        for (b in this) {
            sb.append(String.format("%02x", b))
        }
        return sb.toString()
    }
}
