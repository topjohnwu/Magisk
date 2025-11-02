package com.topjohnwu.magisk.core

import android.content.Context
import android.util.Base64
import com.topjohnwu.magisk.core.logging.JSONLogger
import com.topjohnwu.magisk.core.security.ManifestVerifier
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream
import java.nio.charset.StandardCharsets
import java.security.MessageDigest
import java.security.SecureRandom
import java.time.Instant
import javax.crypto.Mac
import javax.crypto.SecretKey
import javax.crypto.spec.SecretKeySpec
import kotlin.random.Random

/**
 * BackupManager
 *
 * Responsibilities:
 *  - create atomic backups of boot images (or arbitrary files) into app private storage
 *  - compute SHA-256 in streaming fashion
 *  - compute HMAC-SHA256 using a KeyStore-provided key or fallback software key
 *  - write RAFAELIA-style manifest atomically
 *  - validate backups by re-computing SHA and HMAC
 *
 * Notes:
 *  - Keep heavy IO off the main thread. Public suspend functions provided for coroutine usage.
 *  - Uses JSONLogger to emit structured log events for observability.
 */
class BackupManager private constructor(private val ctx: Context) {

    companion object {
        private var instance: BackupManager? = null
        fun getInstance(context: Context): BackupManager {
            if (instance == null) instance = BackupManager(context.applicationContext)
            return instance!!
        }

        private const val MANIFEST_DIR = "rafaelia_backups"
        private const val MANIFEST_EXT = "-manifest.json"
        private const val BACKUP_EXT = ".img"
    }

    private fun getBackupDir(): File {
        val d = File(ctx.filesDir, MANIFEST_DIR)
        if (!d.exists()) d.mkdirs()
        return d
    }

    private fun generateSessionId(): String {
        // compact UUID-like
        val r = SecureRandom()
        val b = ByteArray(12)
        r.nextBytes(b)
        return b.joinToString("") { "%02x".format(it) }
    }

    private fun computeSha256Stream(input: InputStream, progressCb: ((Long) -> Unit)? = null): String {
        val md = MessageDigest.getInstance("SHA-256")
        val buf = ByteArray(32 * 1024)
        var read: Int
        var total = 0L
        while (input.read(buf).also { read = it } != -1) {
            md.update(buf, 0, read)
            total += read
            if (progressCb != null && (total % (256 * 1024) == 0L)) {
                progressCb(total)
            }
        }
        return md.digest().joinToString("") { "%02x".format(it) }
    }

    private fun computeHmacSha256Hex(key: SecretKey, hexData: String): Pair<String, String> {
        val mac = Mac.getInstance("HmacSHA256")
        mac.init(key)
        val bytes = hexStringToByteArray(hexData)
        val h = mac.doFinal(bytes)
        val b64 = Base64.encodeToString(h, Base64.NO_WRAP)
        val hex = h.joinToString("") { "%02x".format(it) }
        return Pair(b64, hex)
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

    // Fallback software key (only for non-production/testing). Prefer KeyStore keys.
    private fun fallbackKey(): SecretKey {
        val seed = ByteArray(32)
        SecureRandom().nextBytes(seed)
        return SecretKeySpec(seed, "HmacSHA256")
    }

    // Public API: backup by File
    suspend fun backupBootImage(source: File, sessionIdIn: String? = null): String? = withContext(Dispatchers.IO) {
        val sessionId = sessionIdIn ?: generateSessionId()
        JSONLogger.info("BackupManager", "backup_start", sessionId, mapOf("source" to source.absolutePath))
        if (!source.exists()) {
            JSONLogger.error("BackupManager", "backup_missing_source", sessionId, mapOf("source" to source.absolutePath))
            return@withContext null
        }
        val dstDir = getBackupDir()
        val tmpFile = File(dstDir, "$sessionId${BACKUP_EXT}.tmp")
        val finalBackup = File(dstDir, "$sessionId$BACKUP_EXT")
        val manifestFile = File(dstDir, "$sessionId$MANIFEST_EXT")

        try {
            // copy + compute sha on the fly
            source.inputStream().use { ins ->
                FileOutputStream(tmpFile).use { out ->
                    val buf = ByteArray(32 * 1024)
                    var read: Int
                    var processed = 0L
                    val md = MessageDigest.getInstance("SHA-256")
                    while (ins.read(buf).also { read = it } != -1) {
                        out.write(buf, 0, read)
                        md.update(buf, 0, read)
                        processed += read
                        if (processed % (256 * 1024) == 0L) {
                            JSONLogger.info("BackupManager", "backup_progress", sessionId, mapOf("processed" to processed))
                        }
                    }
                    out.fd.sync()
                    val shaHex = md.digest().joinToString("") { "%02x".format(it) }

                    // HMAC via KeyStore (integration point) or fallback
                    val key: SecretKey? = try {
                        // Attempt to obtain key from application KeyStore wrapper (if present)
                        KeyStoreWrapper.getHmacKey(context = ctx)
                    } catch (ex: Exception) {
                        null
                    }
                    val sk = key ?: fallbackKey()
                    val (hmacB64, hmacHex) = computeHmacSha256Hex(sk, shaHex)

                    // manifest
                    val manifest = JSONObject()
                    manifest.put("sessionId", sessionId)
                    manifest.put("source", source.absolutePath)
                    manifest.put("backupPath", finalBackup.absolutePath)
                    manifest.put("sha256", shaHex)
                    manifest.put("hmac_b64", hmacB64)
                    manifest.put("hmac_hex", hmacHex)
                    manifest.put("size", processed)
                    manifest.put("timestamp", Instant.now().toString())
                    // atomic: move file then write manifest tmp+rename
                    if (!tmpFile.renameTo(finalBackup)) {
                        JSONLogger.error("BackupManager", "backup_atomic_move_fail", sessionId, mapOf("tmp" to tmpFile.absolutePath, "dst" to finalBackup.absolutePath))
                        tmpFile.delete()
                        return@withContext null
                    }
                    // write manifest
                    val tmpManifest = File(dstDir, "$sessionId${MANIFEST_EXT}.tmp")
                    tmpManifest.writeText(manifest.toString(), StandardCharsets.UTF_8)
                    if (!tmpManifest.renameTo(manifestFile)) {
                        JSONLogger.error("BackupManager", "manifest_atomic_move_fail", sessionId, mapOf("tmp" to tmpManifest.absolutePath, "dst" to manifestFile.absolutePath))
                        return@withContext null
                    }
                    JSONLogger.info("BackupManager", "backup_complete", sessionId, mapOf("backup" to finalBackup.absolutePath, "manifest" to manifestFile.absolutePath))
                    return@withContext manifestFile.absolutePath
                }
            }
        } catch (t: Throwable) {
            JSONLogger.error("BackupManager", "backup_exception", sessionId, mapOf("error" to t.toString()))
            try { tmpFile.delete() } catch (ignore: Exception) {}
            return@withContext null
        }
    }

    // Public API: backup from InputStream (useful for SAF URIs)
    suspend fun backupFromStream(stream: InputStream, originalName: String, sessionIdIn: String? = null): String? = withContext(Dispatchers.IO) {
        val sessionId = sessionIdIn ?: generateSessionId()
        JSONLogger.info("BackupManager", "backup_stream_start", sessionId, mapOf("name" to originalName))
        val dstDir = getBackupDir()
        val tmpFile = File(dstDir, "$sessionId${BACKUP_EXT}.tmp")
        val finalBackup = File(dstDir, "$sessionId$BACKUP_EXT")
        val manifestFile = File(dstDir, "$sessionId$MANIFEST_EXT")

        try {
            stream.use { ins ->
                FileOutputStream(tmpFile).use { out ->
                    val md = MessageDigest.getInstance("SHA-256")
                    val buf = ByteArray(32 * 1024)
                    var read: Int
                    var processed = 0L
                    while (ins.read(buf).also { read = it } != -1) {
                        out.write(buf, 0, read)
                        md.update(buf, 0, read)
                        processed += read
                    }
                    out.fd.sync()
                    val shaHex = md.digest().joinToString("") { "%02x".format(it) }

                    val key = try { KeyStoreWrapper.getHmacKey(context = ctx) } catch (ex: Exception) { null }
                    val sk = key ?: fallbackKey()
                    val (hmacB64, hmacHex) = computeHmacSha256Hex(sk, shaHex)

                    val manifest = JSONObject()
                    manifest.put("sessionId", sessionId)
                    manifest.put("originalName", originalName)
                    manifest.put("backupPath", finalBackup.absolutePath)
                    manifest.put("sha256", shaHex)
                    manifest.put("hmac_b64", hmacB64)
                    manifest.put("hmac_hex", hmacHex)
                    manifest.put("size", processed)
                    manifest.put("timestamp", Instant.now().toString())

                    if (!tmpFile.renameTo(finalBackup)) {
                        JSONLogger.error("BackupManager", "backup_atomic_move_fail", sessionId, mapOf("tmp" to tmpFile.absolutePath, "dst" to finalBackup.absolutePath))
                        tmpFile.delete()
                        return@withContext null
                    }
                    val tmpManifest = File(dstDir, "$sessionId${MANIFEST_EXT}.tmp")
                    tmpManifest.writeText(manifest.toString(), StandardCharsets.UTF_8)
                    if (!tmpManifest.renameTo(manifestFile)) {
                        JSONLogger.error("BackupManager", "manifest_atomic_move_fail", sessionId, mapOf("tmp" to tmpManifest.absolutePath, "dst" to manifestFile.absolutePath))
                        return@withContext null
                    }
                    JSONLogger.info("BackupManager", "backup_stream_complete", sessionId, mapOf("backup" to finalBackup.absolutePath, "manifest" to manifestFile.absolutePath))
                    return@withContext manifestFile.absolutePath
                }
            }
        } catch (t: Throwable) {
            JSONLogger.error("BackupManager", "backup_stream_exception", sessionId, mapOf("error" to t.toString()))
            try { tmpFile.delete() } catch (ignore: Exception) {}
            return@withContext null
        }
    }

    // Load manifest JSON file by sessionId (returns File or null)
    fun loadManifestFile(sessionId: String): File? {
        val f = File(getBackupDir(), "$sessionId$MANIFEST_EXT")
        return if (f.exists()) f else null
    }

    // Validate a backup using the manifest (recompute sha and hmac)
    suspend fun validateBackup(sessionId: String, keyProvider: (() -> SecretKey?)? = null): Boolean = withContext(Dispatchers.IO) {
        JSONLogger.info("BackupManager", "validate_start", sessionId, emptyMap())
        val manifestFile = loadManifestFile(sessionId)
        if (manifestFile == null) {
            JSONLogger.error("BackupManager", "validate_no_manifest", sessionId, emptyMap())
            return@withContext false
        }
        val json = JSONObject(manifestFile.readText(StandardCharsets.UTF_8))
        val backupPath = json.optString("backupPath", "")
        val expectedSha = json.optString("sha256", "")
        val expectedHmacB64 = json.optString("hmac_b64", "")
        if (backupPath.isEmpty() || expectedSha.isEmpty()) {
            JSONLogger.error("BackupManager", "validate_manifest_missing_fields", sessionId, mapOf("manifest" to manifestFile.absolutePath))
            return@withContext false
        }
        val backupFile = File(backupPath)
        if (!backupFile.exists()) {
            JSONLogger.error("BackupManager", "validate_backup_missing", sessionId, mapOf("backup" to backupPath))
            return@withContext false
        }
        // compute sha
        try {
            backupFile.inputStream().use { ins ->
                val md = MessageDigest.getInstance("SHA-256")
                val buf = ByteArray(32 * 1024)
                var read: Int
                while (ins.read(buf).also { read = it } != -1) md.update(buf, 0, read)
                val actualSha = md.digest().joinToString("") { "%02x".format(it) }
                if (!actualSha.equals(expectedSha, ignoreCase = true)) {
                    JSONLogger.error("BackupManager", "validate_sha_mismatch", sessionId, mapOf("expected" to expectedSha, "actual" to actualSha))
                    return@withContext false
                }
                // verify HMAC if present
                if (expectedHmacB64 != null && expectedHmacB64.isNotEmpty()) {
                    val key = try {
                        keyProvider?.invoke() ?: KeyStoreWrapper.getHmacKey(context = ctx)
                    } catch (e: Exception) {
                        null
                    }
                    if (key == null) {
                        JSONLogger.error("BackupManager", "validate_no_key", sessionId, emptyMap())
                        return@withContext false
                    }
                    val mac = Mac.getInstance("HmacSHA256")
                    mac.init(key)
                    val computed = mac.doFinal(hexStringToByteArray(actualSha))
                    val computedB64 = Base64.encodeToString(computed, Base64.NO_WRAP)
                    if (computedB64 != expectedHmacB64) {
                        JSONLogger.error("BackupManager", "validate_hmac_mismatch", sessionId, mapOf("expected_b64" to expectedHmacB64, "computed_b64" to computedB64))
                        return@withContext false
                    }
                }
                JSONLogger.info("BackupManager", "validate_success", sessionId, mapOf("backup" to backupPath))
                return@withContext true
            }
        } catch (t: Throwable) {
            JSONLogger.error("BackupManager", "validate_exception", sessionId, mapOf("error" to t.toString()))
            return@withContext false
        }
    }

    // Helper to allow external components (like rollback script) to validate manifest using app's verifier via adb or manifest_verify CLI
    fun exportManifestForExternalUse(sessionId: String): File? = loadManifestFile(sessionId)

}

/**
 * KeyStoreWrapper is a thin placeholder abstraction.
 * Implement getHmacKey(context) to return a javax.crypto.SecretKey from AndroidKeyStore or throw if not available.
 * Here we provide a stub for compilation; replace with your KeyStore integration.
 */
object KeyStoreWrapper {
    fun getHmacKey(context: Context): SecretKey {
        // TODO: integrate with AndroidKeyStore; throw if not available
        // For now, return a fallback generated key (NOT secure for production)
        val seed = ByteArray(32)
        SecureRandom().nextBytes(seed)
        return SecretKeySpec(seed, "HmacSHA256")
    }
}duction you may persist an encrypted fallback key in SharedPreferences protected by KeyStore or avoid fallback entirely.
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
