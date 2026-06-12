package com.topjohnwu.magisk.test

import android.app.Instrumentation
import android.app.UiAutomation
import android.content.Context
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.uiautomator.UiDevice
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import org.junit.Assert.assertTrue

interface BaseTest {
    val instrumentation: Instrumentation
        get() = InstrumentationRegistry.getInstrumentation()
    val appContext: Context get() = instrumentation.targetContext
    val testContext: Context get() = instrumentation.context
    val uiAutomation: UiAutomation get() = instrumentation.uiAutomation
    val device: UiDevice get() = UiDevice.getInstance(instrumentation)

    companion object {
        fun prerequisite() {
            assertTrue("Should have root access", Shell.getShell().isRoot)
            // Make sure the root service is running
            RootUtils.Connection.await()
        }
    }
}
