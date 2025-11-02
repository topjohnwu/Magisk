package com.topjohnwu.magisk.core

import android.content.Context
import androidx.test.core.app.ApplicationProvider
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.robolectric.RobolectricTestRunner
import org.junit.runner.RunWith
import java.io.File
import java.nio.charset.StandardCharsets

@RunWith(RobolectricTestRunner::class)
class BackupManagerRobolectricTest {

    private lateinit var ctx: Context
    private lateinit var bm: BackupManager

    @Before
    fun setup() {
        ctx = ApplicationProvider.getApplicationContext()
        bm = BackupManager.getInstance(ctx)
    }

    @Test
    fun testBackupAndValidateFromStream() = runBlocking {
        val content = "hello rafaelia test"
        val tmp = File(ctx.cacheDir, "test-boot.img")
        tmp.writeText(content, StandardCharsets.UTF_8)

        // Deterministic test key for unit tests
        val testKeyBase64 = KeyStoreWrapper.createExportableTestKeyForCI()
        val testSecret = KeyStoreWrapper.secretKeyFromRaw(testKeyBase64)

        val manifestPath = bm.backupFromStream(tmp.inputStream(), tmp.name, null)
        assertNotNull("manifest should be created", manifestPath)

        val sessionId = File(manifestPath!!).name.removeSuffix("-manifest.json")
        val ok = bm.validateBackup(sessionId) { testSecret }
        assertTrue("validateBackup should pass with test key", ok)
    }
}
