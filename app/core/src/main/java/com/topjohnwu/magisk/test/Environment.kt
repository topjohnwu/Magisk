package com.topjohnwu.magisk.test

import android.app.Notification
import android.os.Build
import androidx.annotation.Keep
import androidx.core.net.toUri
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.download.DownloadNotifier
import com.topjohnwu.magisk.core.download.DownloadProcessor
import com.topjohnwu.magisk.core.ktx.cachedFile
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.nio.ExtendedFile
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

        // The kernel running on emulators < API 26 does not play well with
        // magic mount. Skip module tests on those legacy platforms.
        fun testModules(): Boolean {
            return Build.VERSION.SDK_INT >= 26
        }

        fun lsposed(): Boolean {
            return Build.VERSION.SDK_INT in 27..34
        }

        fun shamiko(): Boolean {
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

    private fun setupModule01(root: ExtendedFile) {
        val error = "test_01 setup failed"
        val path = root.getChildFile("test_01")

        // Create /system/etc/newfile
        val etc = path.getChildFile("system").getChildFile("etc")
        assertTrue(error, etc.mkdirs())
        assertTrue(error, etc.getChildFile("newfile").createNewFile())

        // Delete /system/bin/screenrecord
        val bin = path.getChildFile("system").getChildFile("bin")
        assertTrue(error, bin.mkdirs())
        assertTrue(error, Shell.cmd("mknod $bin/screenrecord c 0 0").exec().isSuccess)

        // Create an empty zygisk folder
        val module = LocalModule(path)
        assertTrue(error, module.zygiskFolder.mkdir())

        assertTrue(error, Shell.cmd("set_default_perm $path").exec().isSuccess)
    }

    private fun setupModule02(root: ExtendedFile) {
        val error = "test_02 setup failed"
        val path = root.getChildFile("test_02")

        // Create invalid zygisk libraries
        val module = LocalModule(path)
        assertTrue(error, module.zygiskFolder.mkdirs())
        assertTrue(error, module.zygiskFolder.getChildFile("armeabi-v7a.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("arm64-v8a.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("x86.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("x86_64.so").createNewFile())

        assertTrue(error, Shell.cmd("set_default_perm $path").exec().isSuccess)
    }

    private fun setupModule03(root: ExtendedFile) {
        val error = "test_03 setup failed"
        val path = root.getChildFile("test_03")

        // Create a new module but mark is as "remove"
        val module = LocalModule(path)
        assertTrue(error, path.mkdirs())
        assertTrue(error, path.getChildFile("service.sh").createNewFile())
        module.remove = true

        assertTrue(error, Shell.cmd("set_default_perm $path").exec().isSuccess)
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

        if (testModules()) {
            val root = RootUtils.fs.getFile(Const.MODULE_PATH)
            setupModule01(root)
            setupModule02(root)
            setupModule03(root)
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
