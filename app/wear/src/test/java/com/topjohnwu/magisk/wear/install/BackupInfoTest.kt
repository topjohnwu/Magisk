package com.topjohnwu.magisk.wear.install

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class BackupInfoTest {

    private val config =
        "KEEPVERITY=false\nKEEPFORCEENCRYPT=true\nSHA1=abc123\nRECOVERYMODE=false\n"

    @Test
    fun backupExists() {
        var asked: String? = null
        val info = BackupInfo.parse(config) { dir ->
            asked = dir
            true
        }
        assertEquals(BackupInfo("abc123", true), info)
        assertEquals("/data/magisk_backup_abc123", asked)
    }

    @Test
    fun backupMissing() {
        assertEquals(BackupInfo("abc123", false), BackupInfo.parse(config) { false })
    }

    @Test
    fun noSha1() {
        var asked = false
        val info = BackupInfo.parse("KEEPVERITY=false\nRECOVERYMODE=false\n") {
            asked = true
            true
        }
        assertEquals(BackupInfo(null, false), info)
        assertEquals(false, asked)
    }

    @Test
    fun emptySha1() {
        val info = BackupInfo.parse("KEEPVERITY=false\nSHA1=\n") { true }
        assertNull(info.sha1)
        assertEquals(false, info.exists)
    }

    @Test
    fun crlfLine() {
        assertEquals(BackupInfo("abc123", true), BackupInfo.parse("SHA1=abc123\r\n") { true })
    }
}
