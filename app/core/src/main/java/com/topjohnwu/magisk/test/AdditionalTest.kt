package com.topjohnwu.magisk.test

import android.os.ParcelFileDescriptor.AutoCloseInputStream
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.uiautomator.By
import androidx.test.uiautomator.Until
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.utils.RootUtils
import kotlinx.coroutines.runBlocking
import org.junit.After
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
        var expected = 0
        if (Environment.testModules()) expected +=2
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
    fun testModule01() {
        assumeTrue(Environment.testModules())

        val module = modules.find { it.id == "test_01" }
        assertNotNull("test_01 is not installed", module)
        assertTrue(
            "/system/etc/newfile should exist",
            RootUtils.fs.getFile("/system/etc/newfile").exists()
        )
        assertFalse(
            "/system/bin/screenrecord should not exist",
            RootUtils.fs.getFile("/system/bin/screenrecord").exists()
        )
        module!!
        assertTrue("test_01 should be zygisk unloaded", module.zygiskUnloaded)
    }

    @Test
    fun testModule02() {
        assumeTrue(Environment.testModules())

        val module = modules.find { it.id == "test_02" }
        assertNotNull("test_02 is not installed", module)
        module!!
        assertTrue("test_02 should be zygisk unloaded", module.zygiskUnloaded)
    }

    @Test
    fun testModule03() {
        assumeTrue(Environment.testModules())

        assertNull("test_03 should be removed", modules.find { it.id == "test_03" })
    }
}
