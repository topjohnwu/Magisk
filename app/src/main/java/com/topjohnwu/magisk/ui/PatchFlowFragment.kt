package com.topjohnwu.magisk.ui

import android.content.ContentResolver
import android.content.Context
import android.net.Uri
import android.os.Bundle
import android.view.View
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.core.BackupManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.io.InputStream
import java.util.UUID

/**
 * PatchFlowFragment - snippet showing streaming backup via ContentResolver (supports SAF URIs)
 * and blocking progression until validateBackup(sessionId) is true.
 *
 * Integrate into existing fragment: call backupAndProceed(uri) when user selects image.
 */

class PatchFlowFragment : Fragment() {

    private lateinit var backupManager: BackupManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        backupManager = BackupManager(requireContext().applicationContext)
    }

    // Example entry point when user picks a boot image (content:// URI) or File URI
    fun onBootImageSelected(uri: Uri) {
        val sessionId = generateSessionId()
        // launch background backup
        lifecycleScope.launch {
            try {
                val manifest = backupUriToAppStorage(requireContext().contentResolver, uri, sessionId)
                // manifest saved; validate
                val ok = withContext(Dispatchers.IO) { backupManager.validateBackup(sessionId) }
                if (ok) {
                    // proceed to patch step: enable UI transition
                    onBackupSuccess(sessionId, manifest)
                } else {
                    onBackupFailure("Backup validation failed")
                }
            } catch (e: Exception) {
                onBackupFailure(e.message ?: "Backup failed")
            }
        }
    }

    private fun onBackupSuccess(sessionId: String, manifest: String) {
        // Hook into existing flow to enable patch button
        // For example: view?.findViewById<Button>(R.id.button_patch)?.isEnabled = true
    }

    private fun onBackupFailure(reason: String) {
        // Show dialog / toast to user; block progression
    }

    private fun generateSessionId(): String {
        return System.currentTimeMillis().toString() + "-" + UUID.randomUUID().toString().take(8)
    }

    // Streams content URI into an app File inside BackupManager.backupDir location
    private suspend fun backupUriToAppStorage(contentResolver: ContentResolver, uri: Uri, sessionId: String): String {
        return withContext(Dispatchers.IO) {
            // Resolve filename if possible
            val filename = queryFileName(contentResolver, uri) ?: "boot_image"
            // Create a temp file in cache and then call BackupManager which expects a File
            val temp = File(requireContext().cacheDir, "raf_tmp_${sessionId}")
            contentResolver.openInputStream(uri)?.use { input ->
                temp.outputStream().use { out ->
                    val buf = ByteArray(16 * 1024)
                    var read: Int
                    while (input.read(buf).also { read = it } != -1) {
                        out.write(buf, 0, read)
                    }
                    out.fd.sync()
                }
            } ?: throw IllegalArgumentException("Cannot open URI input stream")

            // Now call BackupManager to move into backupDir and create manifest (we assume BackupManager.backupBootImage accepts File)
            val manifest = backupManager.backupBootImage(temp, sessionId, null)
            // Optionally delete temp
            temp.delete()
            manifest.toString()
        }
    }

    private fun queryFileName(resolver: ContentResolver, uri: Uri): String? {
        // Minimal implementation: try to query DISPLAY_NAME via content resolver
        var name: String? = null
        val cursor = resolver.query(uri, arrayOf(android.provider.OpenableColumns.DISPLAY_NAME), null, null, null)
        cursor?.use {
            if (it.moveToFirst()) {
                name = it.getString(0)
            }
        }
        return name
    }
}
