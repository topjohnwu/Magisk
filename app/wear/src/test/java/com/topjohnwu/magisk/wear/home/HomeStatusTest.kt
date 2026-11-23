package com.topjohnwu.magisk.wear.home

import com.topjohnwu.magisk.core.Const
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

// Only of() is tested: touching Info would run its initializer, which needs a real Android
class HomeStatusTest {

    @Test
    fun installed() {
        val status = HomeStatus.of("31.1-wear", 31100, rooted = true, zygisk = true, ramdisk = false)
        assertTrue(status.installed)
        assertEquals("31.1-wear", status.version)
        assertEquals(31100, status.versionCode)
        assertTrue(status.zygisk)
        assertFalse(status.ramdisk)
    }

    @Test
    fun daemonNotRunning() {
        val status = HomeStatus.of("", -1, rooted = false, zygisk = false, ramdisk = true)
        assertFalse(status.installed)
        assertNull(status.version)
        assertNull(status.versionCode)
        assertTrue(status.ramdisk)
    }

    @Test
    fun unsupportedVersion() {
        val status = HomeStatus.of(
            "21.4", Const.Version.MIN_VERCODE - 1, rooted = true, zygisk = false, ramdisk = false
        )
        assertFalse(status.installed)
        assertNull(status.version)
        assertNull(status.versionCode)
    }

    @Test
    fun notRooted() {
        val status = HomeStatus.of("31.1-wear", 31100, rooted = false, zygisk = false, ramdisk = false)
        assertFalse(status.installed)
        assertNull(status.version)
        assertNull(status.versionCode)
    }
}
