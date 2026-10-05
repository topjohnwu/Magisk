package com.topjohnwu.magisk.test

import android.os.Build
import android.os.ParcelFileDescriptor.AutoCloseInputStream
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.model.su.SuPolicy
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith

@Keep
@RunWith(AndroidJUnit4::class)
class MagiskAppTest : BaseTest {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() = BaseTest.prerequisite()
    }

    @Test
    fun testZygisk() {
        assertTrue("Zygisk should be enabled", Info.isZygiskEnabled)
    }

    @Test
    fun testSu() {
        // Explicitly grant shell su access
        val policy = SuPolicy(
            uid = 2000,
            policy = SuPolicy.ALLOW,
            logging = false,
            notification = false,
            remain = 0L
        )
        runBlocking {
            ServiceLocator.policyDB.update(policy)
        }

        // Try to call su from ADB shell
        val cmd = if (Build.VERSION.SDK_INT < 24) {
            // API 23 runs executeShellCommand as root
            "/system/xbin/su 2000 su -c id"
        } else {
            "su -c id"
        }
        val pfd = uiAutomation.executeShellCommand(cmd)

        // Check that root access is granted
        AutoCloseInputStream(pfd).reader().use {
            assertTrue(
                "Cannot grant root permission from shell",
                it.readText().contains("uid=0")
            )
        }
    }
}
