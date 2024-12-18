package com.topjohnwu.magisk.test

import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith

@Keep
@RunWith(AndroidJUnit4::class)
class MagiskAppTest {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() {
            assertTrue("Should have root access", Shell.getShell().isRoot)
            // Make sure the root service is running
            RootUtils.Connection.await()
        }
    }

    @Test
    fun testZygisk() {
        assertTrue("Zygisk should be enabled", Info.isZygiskEnabled)
    }
}
