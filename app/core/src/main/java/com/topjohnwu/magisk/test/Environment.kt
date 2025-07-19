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
import java.io.PrintStream

@Keep
@RunWith(AndroidJUnit4::class)
class Environment : BaseTest {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() = BaseTest.prerequisite()

        // The kernel running on emulators < API 26 does not play well with
        // magic mount. Skip mount_test on those legacy platforms.
        fun mount(): Boolean {
            return Build.VERSION.SDK_INT >= 26
        }

        // It is possible that there are no suitable preinit partition to use
        fun preinit(): Boolean {
            return Shell.cmd("magisk --preinit-device").exec().isSuccess
        }

        fun lsposed(): Boolean {
            return Build.VERSION.SDK_INT in 27..34
        }

        fun shamiko(): Boolean {
            return Build.VERSION.SDK_INT >= 27
        }

        private const val MODULE_ERROR = "Module zip processing incorrect"
        const val MOUNT_TEST = "mount_test"
        const val SEPOLICY_RULE = "sepolicy_rule"
        const val INVALID_ZYGISK = "invalid_zygisk"
        const val REMOVE_TEST = "remove_test"
        const val EMPTY_ZYGISK = "empty_zygisk"
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

    private fun setupMountTest(root: ExtendedFile) {
        val error = "$MOUNT_TEST setup failed"
        val path = root.getChildFile(MOUNT_TEST)

        // Create /system/fonts/newfile
        val etc = path.getChildFile("system").getChildFile("fonts")
        assertTrue(error, etc.mkdirs())
        assertTrue(error, etc.getChildFile("newfile").createNewFile())

        // Create /system/app/EasterEgg/.replace
        val egg = path.getChildFile("system").getChildFile("app").getChildFile("EasterEgg")
        assertTrue(error, egg.mkdirs())
        assertTrue(error, egg.getChildFile(".replace").createNewFile())

        // Create /system/app/EasterEgg/newfile
        assertTrue(error, egg.getChildFile("newfile").createNewFile())

        // Delete /system/bin/screenrecord
        val bin = path.getChildFile("system").getChildFile("bin")
        assertTrue(error, bin.mkdirs())
        assertTrue(error, Shell.cmd("mknod $bin/screenrecord c 0 0").exec().isSuccess)

        assertTrue(error, Shell.cmd("set_default_perm $path").exec().isSuccess)
    }

    private fun setupSystemlessHost() {
        assertTrue("hosts setup failed", Shell.cmd("add_hosts_module").exec().isSuccess)
    }

    private fun setupSepolicyRuleModule(root: ExtendedFile) {
        val error = "$SEPOLICY_RULE setup failed"
        val path = root.getChildFile(SEPOLICY_RULE)
        assertTrue(error, path.mkdirs())

        // Add sepolicy patch
        PrintStream(path.getChildFile("sepolicy.rule").newOutputStream()).use {
            it.println("type magisk_test domain")
        }

        assertTrue(error, Shell.cmd(
            "set_default_perm $path",
            "copy_preinit_files"
        ).exec().isSuccess)
    }

    private fun setupEmptyZygiskModule(root: ExtendedFile) {
        val error = "$EMPTY_ZYGISK setup failed"
        val path = root.getChildFile(EMPTY_ZYGISK)

        // Create an empty zygisk folder
        val module = LocalModule(path)
        assertTrue(error, module.zygiskFolder.mkdirs())
    }

    private fun setupInvalidZygiskModule(root: ExtendedFile) {
        val error = "$INVALID_ZYGISK setup failed"
        val path = root.getChildFile(INVALID_ZYGISK)

        // Create invalid zygisk libraries
        val module = LocalModule(path)
        assertTrue(error, module.zygiskFolder.mkdirs())
        assertTrue(error, module.zygiskFolder.getChildFile("armeabi-v7a.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("arm64-v8a.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("x86.so").createNewFile())
        assertTrue(error, module.zygiskFolder.getChildFile("x86_64.so").createNewFile())

        assertTrue(error, Shell.cmd("set_default_perm $path").exec().isSuccess)
    }

    private fun setupRemoveModule(root: ExtendedFile) {
        val error = "$REMOVE_TEST setup failed"
        val path = root.getChildFile(REMOVE_TEST)

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

        val root = RootUtils.fs.getFile(Const.MODULE_PATH)
        if (mount()) { setupMountTest(root) }
        if (preinit()) { setupSepolicyRuleModule(root) }
        setupSystemlessHost()
        setupEmptyZygiskModule(root)
        setupInvalidZygiskModule(root)
        setupRemoveModule(root)
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
