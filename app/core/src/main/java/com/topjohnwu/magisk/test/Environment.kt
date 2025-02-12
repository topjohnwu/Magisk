package com.topjohnwu.magisk.test

import android.app.Notification
import android.os.Build
import androidx.annotation.Keep
import androidx.core.net.toUri
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.download.DownloadNotifier
import com.topjohnwu.magisk.core.download.DownloadProcessor
import com.topjohnwu.magisk.core.ktx.cachedFile
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.superuser.CallbackList
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import timber.log.Timber

@Keep
@RunWith(AndroidJUnit4::class)
class Environment : BaseTest {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() = BaseTest.prerequisite()

        fun lsposed(): Boolean {
            return Build.VERSION.SDK_INT >= 27 && Build.VERSION.SDK_INT <= 34
        }
    }

    object TimberLog : CallbackList<String>(Runnable::run) {
        override fun onAddElement(e: String) {
            Timber.i(e)
        }
    }

    @Test
    fun setupEnvironment() {
        runBlocking {
            assertTrue(
                "Magisk setup failed",
                MagiskInstaller.Emulator(TimberLog, TimberLog).exec()
            )
        }

        if (lsposed()) {
            val notify = object : DownloadNotifier {
                override val context = appContext
                override fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit) {}
            }
            val processor = DownloadProcessor(notify)
            val zip = appContext.cachedFile("lsposed.zip")
            runBlocking {
                testContext.assets.open("lsposed.zip").use {
                    processor.handleModule(it, zip.toUri())
                }
                assertTrue(
                    "LSPosed installation failed",
                    FlashZip(zip.toUri(), TimberLog, TimberLog).exec()
                )
            }
        }
    }

    @Test
    fun setupAppHide() {
        runBlocking {
            assertTrue(
                "App hiding failed",
                AppMigration.patchAndHide(
                    context = appContext,
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
                AppMigration.restoreApp(appContext)
            )
        }
    }
}
