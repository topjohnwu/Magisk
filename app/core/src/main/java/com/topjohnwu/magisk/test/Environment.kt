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
import org.apache.commons.compress.archivers.zip.ZipFile
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import timber.log.Timber
import java.io.File

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

        private fun shamiko(): Boolean {
            return Build.VERSION.SDK_INT >= 27
        }

        private const val MODULE_ERROR = "Module zip processing incorrect"
    }

    object TimberLog : CallbackList<String>(Runnable::run) {
        override fun onAddElement(e: String) {
            Timber.i(e)
        }
    }

    private fun checkModuleZip(file: File) {
        // Make sure module processing is correct
        ZipFile.Builder().setFile(file).get().use { zip ->
            val meta = zip.entries
                .asSequence()
                .filter { it.name.startsWith("META-INF") }
                .toMutableList()
            assertEquals(MODULE_ERROR, 6, meta.size)

            val binary = zip.getInputStream(
                zip.getEntry("META-INF/com/google/android/update-binary")
            ).use { it.readBytes() }
            val ref = appContext.assets.open("module_installer.sh").use { it.readBytes() }
            assertArrayEquals(MODULE_ERROR, ref, binary)

            val script = zip.getInputStream(
                zip.getEntry("META-INF/com/google/android/updater-script")
            ).use { it.readBytes() }
            assertArrayEquals(MODULE_ERROR, "#MAGISK\n".toByteArray(), script)
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

        val notify = object : DownloadNotifier {
            override val context = appContext
            override fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit) {}
        }
        val processor = DownloadProcessor(notify)

        val shamiko = appContext.cachedFile("shamiko.zip")
        runBlocking {
            testContext.assets.open("shamiko.zip").use {
                processor.handleModule(it, shamiko.toUri())
            }
            checkModuleZip(shamiko)
            if (shamiko()) {
                assertTrue(
                    "Shamiko installation failed",
                    FlashZip(shamiko.toUri(), TimberLog, TimberLog).exec()
                )
            }
        }

        val lsp = appContext.cachedFile("lsposed.zip")
        runBlocking {
            testContext.assets.open("lsposed.zip").use {
                processor.handleModule(it, lsp.toUri())
            }
            checkModuleZip(lsp)
            if (lsposed()) {
                assertTrue(
                    "LSPosed installation failed",
                    FlashZip(lsp.toUri(), TimberLog, TimberLog).exec()
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
