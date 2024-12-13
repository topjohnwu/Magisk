package com.topjohnwu.magisk.core

import androidx.annotation.Keep
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import timber.log.Timber

/**
 * We implement all test logic here and mark it with @Keep so that our instrumentation package
 * can properly run tests on fully obfuscated release APKs.
 */
@Keep
object TestImpl {

    fun before() {
        assertTrue("Should have root access", Shell.getShell().isRoot)
        // Make sure the root service is running
        RootUtils.Connection.await()
    }

    object LogList : CallbackList<String>(Runnable::run) {
        override fun onAddElement(e: String) {
            Timber.i(e)
        }
    }

    fun setupMagisk() {
        runBlocking {
            MagiskInstaller.Emulator(LogList, LogList).exec()
        }
    }

    fun setupShellGrantTest() {
        // Clear existing grant for ADB shell
        runBlocking {
            ServiceLocator.policyDB.delete(2000)
            Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
            Config.prefs.edit().commit()
        }
    }

    fun testZygisk() {
        assertTrue("Zygisk should be enabled", Info.isZygiskEnabled)
    }
}
