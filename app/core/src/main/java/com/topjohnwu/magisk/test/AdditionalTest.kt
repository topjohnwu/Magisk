package com.topjohnwu.magisk.test

import android.os.ParcelFileDescriptor.AutoCloseInputStream
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.uiautomator.By
import androidx.test.uiautomator.Until
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.test.Environment.Companion.EMPTY_ZYGISK
import com.topjohnwu.magisk.test.Environment.Companion.INVALID_ZYGISK
import com.topjohnwu.magisk.test.Environment.Companion.MOUNT_TEST
import com.topjohnwu.magisk.test.Environment.Companion.REMOVE_TEST
import com.topjohnwu.magisk.test.Environment.Companion.SEPOLICY_RULE
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.TimeUnit
import java.util.regex.Pattern

@Keep
@RunWith(AndroidJUnit4::class)
class AdditionalTest : BaseTest {

    companion object {
        private const val SHELL_PKG = "com.android.shell"
        private const val LSPOSED_CATEGORY = "org.lsposed.manager.LAUNCH_MANAGER"
        private const val LSPOSED_PKG = "org.lsposed.manager"

        private lateinit var modules: List<LocalModule>

        @BeforeClass
        @JvmStatic
        fun before() {
            BaseTest.prerequisite()
            runBlocking {
                modules = LocalModule.installed()
            }
        }
    }

    @After
    fun teardown() {
        device.pressHome()
    }

    @Test
    fun testModuleCount() {
        var expected = 3
        if (Environment.mount()) expected++
        if (Environment.preinit()) expected++
        if (Environment.lsposed()) expected++
        if (Environment.shamiko()) expected++
        assertEquals("Module count incorrect", expected, modules.size)
    }

    @Test
    fun testLsposed() {
        assumeTrue(Environment.lsposed())

        val module = modules.find { it.id == "zygisk_lsposed" }
        assertNotNull("zygisk_lsposed is not installed", module)
        module!!
        assertFalse("zygisk_lsposed is not enabled", module.zygiskUnloaded)

        // Launch lsposed manager to ensure the module is active
        uiAutomation.executeShellCommand(
            "am start -c $LSPOSED_CATEGORY $SHELL_PKG/.BugreportWarningActivity"
        ).let { pfd -> AutoCloseInputStream(pfd).use { it.readBytes() } }

        val pattern = Pattern.compile("$LSPOSED_PKG:id/.*")
        assertNotNull(
            "LSPosed manager launch failed",
            device.wait(Until.hasObject(By.res(pattern)), TimeUnit.SECONDS.toMillis(10))
        )
    }

    @Test
    fun testModuleMount() {
        assumeTrue(Environment.mount())

        assertNotNull("$MOUNT_TEST is not installed", modules.find { it.id == MOUNT_TEST })
        assertTrue(
            "/system/fonts/newfile should exist",
            RootUtils.fs.getFile("/system/fonts/newfile").exists()
        )
        assertFalse(
            "/system/bin/screenrecord should not exist",
            RootUtils.fs.getFile("/system/bin/screenrecord").exists()
        )
        val egg = RootUtils.fs.getFile("/system/app/EasterEgg").list() ?: arrayOf()
        assertArrayEquals(
            "/system/app/EasterEgg should be replaced",
            egg,
            arrayOf("newfile")
        )
    }

    @Test
    fun testSepolicyRule() {
        assumeTrue(Environment.preinit())

        assertNotNull("$SEPOLICY_RULE is not installed", modules.find { it.id == SEPOLICY_RULE })
        assertTrue(
            "Module sepolicy.rule is not applied",
            Shell.cmd("magiskpolicy --print-rules | grep -q magisk_test").exec().isSuccess
        )
    }

    @Test
    fun testEmptyZygiskModule() {
        val module = modules.find { it.id == EMPTY_ZYGISK }
        assertNotNull("$EMPTY_ZYGISK is not installed", module)
        module!!
        assertTrue("$EMPTY_ZYGISK should be zygisk unloaded", module.zygiskUnloaded)
    }

    @Test
    fun testInvalidZygiskModule() {
        val module = modules.find { it.id == INVALID_ZYGISK }
        assertNotNull("$INVALID_ZYGISK is not installed", module)
        module!!
        assertTrue("$INVALID_ZYGISK should be zygisk unloaded", module.zygiskUnloaded)
    }

    @Test
    fun testRemoveModule() {
        assertNull("$REMOVE_TEST should be removed", modules.find { it.id == REMOVE_TEST })
    }
}
