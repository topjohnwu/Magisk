package com.topjohnwu.magisk.test

import android.content.Intent
import android.content.IntentFilter
import android.os.Build
import android.os.ParcelFileDescriptor.AutoCloseInputStream
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.model.su.SuPolicy
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.TimeUnit

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
    fun testSuRequest() {
        // Bypass the need to actually show a dialog
        Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
        Config.prefs.edit().commit()

        // Inject an undetermined + mute logging policy for ADB shell
        val policy = SuPolicy(
            uid = 2000,
            logging = false,
            notification = false,
            remain = 0L
        )
        runBlocking {
            ServiceLocator.policyDB.update(policy)
        }

        val filter = IntentFilter(Intent.ACTION_VIEW)
        filter.addCategory(Intent.CATEGORY_DEFAULT)
        val monitor = instrumentation.addMonitor(filter, null, false)

        // Try to call su from ADB shell
        val cmd = if (Build.VERSION.SDK_INT < 24) {
            // API 23 runs executeShellCommand as root
            "/system/xbin/su 2000 su -c id"
        } else {
            "su -c id"
        }
        val pfd = uiAutomation.executeShellCommand(cmd)

        // Make sure SuRequestActivity is launched
        val suRequest = monitor.waitForActivityWithTimeout(TimeUnit.SECONDS.toMillis(10))
        assertNotNull("SuRequestActivity is not launched", suRequest)

        // Check that the request went through
        AutoCloseInputStream(pfd).reader().use {
            assertTrue(
                "Cannot grant root permission from shell",
                it.readText().contains("uid=0")
            )
        }

        // Check that the database is updated
        runBlocking {
            val policy = ServiceLocator.policyDB.fetch(2000)
                ?: throw AssertionError("PolicyDB is invalid")
            assertEquals("Policy for shell is incorrect", SuPolicy.ALLOW, policy.policy)
        }
    }
}
