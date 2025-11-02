package com.topjohnwu.magisk.ui

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.ProgressBar
import android.widget.TextView
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.core.BackupManager
import com.topjohnwu.magisk.core.logging.JSONLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream

/**
 * PatchFlowFragment
 *
 * - Provides a minimal UI flow to pick a boot image (SAF/intent) and run BackupManager.backupFromStream
 * - Emits structured logs to JSONLogger for observability
 * - Shows simple progress UI hooks (you should replace placeholders with real layout IDs)
 */
class PatchFlowFragment : Fragment() {

    private val REQUEST_CODE_PICK = 1234
    private lateinit var pickButton: Button
    private lateinit var statusText: TextView
    private lateinit var progressBar: ProgressBar

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        // Replace with your real layout inflation
        val root = inflater.inflate(android.R.layout.simple_list_item_1, container, false)
        pickButton = Button(requireContext()).apply { text = "Select boot image" }
        statusText = TextView(requireContext())
        progressBar = ProgressBar(requireContext(), null, android.R.attr.progressBarStyleHorizontal)
        (root as ViewGroup).addView(pickButton)
        root.addView(statusText)
        root.addView(progressBar)
        pickButton.setOnClickListener { openFilePicker() }
        return root
    }

    private fun openFilePicker() {
        JSONLogger.info("PatchFlowFragment", "open_picker", null, emptyMap())
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
        }
        startActivityForResult(intent, REQUEST_CODE_PICK)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_CODE_PICK && resultCode == Activity.RESULT_OK) {
            val uri: Uri? = data?.data
            if (uri != null) {
                lifecycleScope.launch { handleUri(uri) }
            } else {
                JSONLogger.error("PatchFlowFragment", "picker_no_uri", null, emptyMap())
                Toast.makeText(requireContext(), "No file selected", Toast.LENGTH_SHORT).show()
            }
        }
    }

    private suspend fun handleUri(uri: Uri) {
        val sessionId = generateSessionId()
        JSONLogger.info("PatchFlowFragment", "file_selected", sessionId, mapOf("uri" to uri.toString()))
        showProgress(true, "Preparing backup...")
        val ok = withContext(Dispatchers.IO) {
            try {
                // Copy URI to temp file within app cache and pass InputStream to BackupManager
                val tmp = File(requireContext().cacheDir, "rafaelia-${sessionId}.img")
                requireContext().contentResolver.openInputStream(uri)?.use { ins ->
                    FileOutputStream(tmp).use { out ->
                        val buf = ByteArray(32 * 1024)
                        var read: Int
                        while (ins.read(buf).also { read = it } != -1) out.write(buf, 0, read)
                        out.fd.sync()
                    }
                } ?: run {
                    JSONLogger.error("PatchFlowFragment", "uri_open_fail", sessionId, mapOf("uri" to uri.toString()))
                    return@withContext false
                }
                // Call backup
                val bm = BackupManager.getInstance(requireContext())
                val manifestPath = bm.backupBootImage(source = tmp, sessionIdIn = sessionId)
                if (manifestPath == null) {
                    JSONLogger.error("PatchFlowFragment", "backup_call_failed", sessionId, mapOf("tmp" to tmp.absolutePath))
                    tmp.delete()
                    return@withContext false
                }
                JSONLogger.info("PatchFlowFragment", "backup_call_success", sessionId, mapOf("manifest" to manifestPath))
                tmp.delete()
                return@withContext true
            } catch (t: Throwable) {
                JSONLogger.error("PatchFlowFragment", "backup_unexpected", sessionId, mapOf("error" to t.toString()))
                return@withContext false
            }
        }
        showProgress(false)
        if (ok) {
            JSONLogger.info("PatchFlowFragment", "ui_backup_success", sessionId, emptyMap())
            Toast.makeText(requireContext(), "Backup finished", Toast.LENGTH_SHORT).show()
            onBackupSuccess(sessionId)
        } else {
            JSONLogger.error("PatchFlowFragment", "ui_backup_failure", sessionId, emptyMap())
            Toast.makeText(requireContext(), "Backup failed — check logs", Toast.LENGTH_LONG).show()
            onBackupFailure(sessionId)
        }
    }

    private fun showProgress(show: Boolean, message: String? = null) {
        progressBar.visibility = if (show) View.VISIBLE else View.GONE
        statusText.text = message ?: ""
    }

    private fun generateSessionId(): String {
        val sb = StringBuilder()
        repeat(12) { sb.append("%02x".format((0..255).random())) }
        return sb.toString()
    }

    // Hooks to integrate with host activity or tests
    protected open fun onBackupSuccess(sessionId: String) {}
    protected open fun onBackupFailure(sessionId: String) {}
}
