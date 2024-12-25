package com.topjohnwu.magisk.test

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.ParcelFileDescriptor.AutoCloseInputStream
import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

@Keep
@RunWith(AndroidJUnit4::class)
class AppMigrationTest {

    companion object {
        private const val APP_PKG = "com.topjohnwu.magisk"
        private const val STUB_PKG = "repackaged.$APP_PKG"
        private const val RECEIVER_TIMEOUT = 20L
    }

    private val instrumentation get() = InstrumentationRegistry.getInstrumentation()
    private val context get() = instrumentation.context
    private val uiAutomation get() = instrumentation.uiAutomation
    private val registeredReceivers = mutableListOf<BroadcastReceiver>()

    class PackageRemoveMonitor(
        context: Context,
        private val packageName: String
    ) : BroadcastReceiver() {

        val latch = CountDownLatch(1)

        init {
            val filter = IntentFilter(Intent.ACTION_PACKAGE_REMOVED)
            filter.addDataScheme("package")
            context.registerReceiver(this, filter)
        }

        override fun onReceive(context: Context, intent: Intent) {
            if (intent.action != Intent.ACTION_PACKAGE_REMOVED)
                return
            val data = intent.data ?: return
            val pkg = data.schemeSpecificPart
            if (pkg == packageName) latch.countDown()
        }
    }

    @After
    fun tearDown() {
        registeredReceivers.forEach(context::unregisterReceiver)
    }

    private fun testAppMigration(pkg: String, method: String) {
        val receiver = PackageRemoveMonitor(context, pkg)
        registeredReceivers.add(receiver)

        // Trigger the test to run migration
        val pfd = uiAutomation.executeShellCommand(
            "am instrument -w --user 0 -e class .Environment#$method " +
                "$pkg.test/${AppTestRunner::class.java.name}"
        )
        val output = AutoCloseInputStream(pfd).reader().use { it.readText() }
        assertTrue("$method failed, inst out: $output", output.contains("OK ("))

        // Wait for migration to complete
        assertTrue(
            "$pkg uninstallation failed",
            receiver.latch.await(RECEIVER_TIMEOUT, TimeUnit.SECONDS)
        )
    }

    @Test
    fun testAppHide() {
        testAppMigration(APP_PKG, "setupAppHide")
    }

    @Test
    fun testAppRestore() {
        testAppMigration(STUB_PKG, "setupAppRestore")
    }
}
