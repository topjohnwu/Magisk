package com.topjohnwu.magisk.ui

import android.content.Context
import android.net.Uri
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.BackupManager
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.io.FileOutputStream

@RunWith(AndroidJUnit4::class)
class PatchFlowFragmentTest {

    @Test
    fun testBackupFlowWithLocalFileUri() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        val bm = BackupManager(context)

        // Create a small dummy "boot image" in cache to simulate selection via SAF
        val temp = File(context.cacheDir, "test_boot.img")
        if (temp.exists()) temp.delete()
        FileOutputStream(temp).use { out ->
            val data = ByteArray(1024)
            for (i in data.indices) data[i] = (i % 256).toByte()
            out.write(data)
            out.fd.sync()
        }

        val sessionId = "test-" + System.currentTimeMillis()
        // Call directly the manager to simulate flow
        val manifest = runBlocking {
            bm.backupBootImage(temp, sessionId, buildId = "test-build")
        }
        // manifest should exist and contain sha256
        val saved = bm.loadManifest(sessionId)
        assertTrue(saved != null)
        assertTrue(saved!!.has("sha256"))

        // validateBackup should return true
        val ok = runBlocking { bm.validateBackup(sessionId) }
        assertTrue(ok)

        // cleanup
        temp.delete()
    }
}
