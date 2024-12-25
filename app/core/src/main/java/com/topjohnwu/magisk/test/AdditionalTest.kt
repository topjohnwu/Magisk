package com.topjohnwu.magisk.test

import android.app.UiAutomation
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.uiautomator.By
import androidx.test.uiautomator.UiDevice
import androidx.test.uiautomator.Until
import org.junit.After
import org.junit.Assert.assertNotNull
import org.junit.Assume.assumeTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.TimeUnit
import java.util.regex.Pattern

@Keep
@RunWith(AndroidJUnit4::class)
class AdditionalTest {

    companion object {
        private const val SHELL_PKG = "com.android.shell"
        private const val LSPOSED_CATEGORY = "org.lsposed.manager.LAUNCH_MANAGER"
        private const val LSPOSED_PKG = "org.lsposed.manager"
    }

    private lateinit var uiAutomation: UiAutomation
    private lateinit var device: UiDevice

    @Before
    fun setup() {
        val inst = InstrumentationRegistry.getInstrumentation()
        uiAutomation = inst.uiAutomation
        device = UiDevice.getInstance(inst)
    }

    @After
    fun teardown() {
        device.pressHome()
    }

    @Test
    fun testLaunchLsposedManager() {
        assumeTrue(Environment.lsposed())

        uiAutomation.executeShellCommand(
            "am start -c $LSPOSED_CATEGORY $SHELL_PKG/.BugreportWarningActivity"
        )
        val pattern = Pattern.compile("$LSPOSED_PKG:id/.*")
        assertNotNull(
            "LSPosed manager launch failed",
            device.wait(Until.hasObject(By.res(pattern)), TimeUnit.SECONDS.toMillis(10))
        )
    }
}
