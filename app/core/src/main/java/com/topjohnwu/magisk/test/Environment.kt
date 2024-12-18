package com.topjohnwu.magisk.test

import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.superuser.CallbackList
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import timber.log.Timber

@Keep
@RunWith(AndroidJUnit4::class)
class Environment {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() = MagiskAppTest.before()
    }

    @Test
    fun setupMagisk() {
        val log = object : CallbackList<String>(Runnable::run) {
            override fun onAddElement(e: String) {
                Timber.i(e)
            }
        }
        runBlocking {
            assertTrue(
                "Magisk setup failed",
                MagiskInstaller.Emulator(log, log).exec()
            )
        }
    }

    @Test
    fun setupShellGrantTest() {
        runBlocking {
            // Inject an undetermined + mute logging policy for ADB shell
            val policy = SuPolicy(
                uid = 2000,
                logging = false,
                notification = false,
                until = 0L
            )
            ServiceLocator.policyDB.update(policy)
            // Bypass the need to actually show a dialog
            Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
            Config.prefs.edit().commit()
        }
    }

    @Test
    fun setupAppHide() {
        runBlocking {
            assertTrue(
                "App hiding failed",
                AppMigration.patchAndHide(
                    context = InstrumentationRegistry.getInstrumentation().targetContext,
                    label = "Settings",
                    pkg = "repackaged.$APP_PACKAGE_NAME"
                )
            )
        }
    }

    @Test
    fun setupAppRestore() {
        runBlocking {
            assertTrue(
                "App restoration failed",
                AppMigration.restoreApp(
                    context = InstrumentationRegistry.getInstrumentation().targetContext
                )
            )
        }
    }
}
