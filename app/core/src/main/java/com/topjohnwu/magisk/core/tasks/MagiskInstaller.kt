package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import android.os.ParcelFileDescriptor
import android.os.Process
import android.system.Os
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.AppApkPath
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.copyAll
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.magisk.core.utils.DummyList
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.openFd
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.apache.commons.compress.archivers.zip.ZipFile
import timber.log.Timber
import java.io.Closeable
import java.io.File
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.security.SecureRandom
import java.util.Locale
import java.util.concurrent.atomic.AtomicBoolean

interface WholeFilePatcher : Closeable {
    @Throws(IOException::class)
    suspend fun start(input: InputStream): ExtendedFile

    @Throws(IOException::class)
    suspend fun finish(patched: ExtendedFile)

    // Release resources if the patching process is aborted before finish
    override fun close() {}
}

interface ImageExtractor {
    @Throws(IOException::class)
    suspend fun extract(channel: DataChannel)
}

abstract class MagiskInstallImpl protected constructor(
    protected val console: MutableList<String>,
    private val logs: MutableList<String>
) {

    private lateinit var installDir: ExtendedFile
    private lateinit var targetImage: ExtendedFile

    private val shell = Shell.getShell()
    private val useRootFs = shell.isRoot && Info.noDataExec
    protected val context get() = ServiceLocator.deContext

    private val rootFS get() = RootUtils.fs
    private val localFS get() = FileSystemManager.getLocal()

    private val destName: String by lazy {
        if (Config.randName) {
            val alpha = "abcdefghijklmnopqrstuvwxyz"
            val alphaNum = "$alpha${alpha.uppercase(Locale.ROOT)}0123456789"
            val random = SecureRandom()
            StringBuilder("magisk_patched-${BuildConfig.APP_VERSION_CODE}_").run {
                for (i in 1..5) {
                    append(alphaNum[random.nextInt(alphaNum.length)])
                }
                toString()
            }
        } else {
            "magisk_patched"
        }
    }

    private fun findImage(slot: String): Boolean {
        val cmd =
            "RECOVERYMODE=${Config.recovery} " +
            "VENDORBOOT=${Info.isVendorBoot} " +
            "SLOT=$slot " +
            "find_boot_image; echo \$BOOTIMAGE"
        val bootPath = ("($cmd)").fsh()
        if (bootPath.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        targetImage = rootFS.getFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun findImage(): Boolean {
        return findImage(Info.slot)
    }

    private fun findSecondary(): Boolean {
        val slot = if (Info.slot == "_a") "_b" else "_a"
        console.add("- Target slot: $slot")
        return findImage(slot)
    }

    private suspend fun extractFiles(): Boolean {
        console.add("- Device platform: ${Const.CPU_ABI}")
        console.add("- Installing: ${BuildConfig.APP_VERSION_NAME} (${BuildConfig.APP_VERSION_CODE})")

        installDir = localFS.getFile(context.filesDir.parent, "install")
        installDir.deleteRecursively()
        installDir.mkdirs()

        try {
            // Extract binaries
            if (isRunningAsStub) {
                ZipFile.builder().setFile(StubApk.current(context)).get().use { zf ->
                    zf.entries.asSequence().filter {
                        !it.isDirectory && it.name.startsWith("lib/${Const.CPU_ABI}/")
                    }.forEach {
                        val n = it.name.substring(it.name.lastIndexOf('/') + 1)
                        val name = n.substring(3, n.length - 3)
                        val dest = File(installDir, name)
                        zf.getInputStream(it).writeTo(dest)
                        dest.setExecutable(true)
                    }

                    val abi32 = Const.CPU_ABI_32
                    if (Process.is64Bit() && abi32 != null) {
                        val entry = zf.getEntry("lib/$abi32/libmagisk.so")
                        if (entry != null) {
                            val magisk32 = File(installDir, "magisk32")
                            zf.getInputStream(entry).writeTo(magisk32)
                        }
                    }
                }
            } else {
                val info = context.applicationInfo
                val libs = File(info.nativeLibraryDir).listFiles { _, name ->
                    name.startsWith("lib") && name.endsWith(".so")
                } ?: emptyArray()

                for (lib in libs) {
                    val name = lib.name.substring(3, lib.name.length - 3)
                    Os.symlink(lib.path, "$installDir/$name")
                }

                // Also extract magisk32 on 64-bit devices that supports 32-bit
                val abi32 = Const.CPU_ABI_32
                if (Process.is64Bit() && abi32 != null) {
                    val name = "lib/$abi32/libmagisk.so"
                    val entry = javaClass.classLoader!!.getResourceAsStream(name)
                    if (entry != null) {
                        val magisk32 = File(installDir, "magisk32")
                        entry.writeTo(magisk32)
                    }
                }
            }

            // Extract scripts
            for (script in listOf("util_functions.sh", "boot_patch.sh", "addon.d.sh", "stub.apk")) {
                val dest = File(installDir, script)
                context.assets.open(script).writeTo(dest)
            }
        } catch (e: Exception) {
            console.add("! Unable to extract files")
            Timber.e(e)
            return false
        }

        if (useRootFs) {
            // Move everything to tmpfs to workaround Samsung bullshit
            rootFS.getFile(Const.TMPDIR).also {
                arrayOf(
                    "rm -rf $it",
                    "mkdir -p $it",
                    "cp_readlink $installDir $it",
                    "rm -rf $installDir"
                ).sh()
                installDir = it
            }
        }

        return true
    }

    private suspend fun InputStream.copyAndCloseOut(out: OutputStream) =
        out.use { copyAll(it, 1024 * 1024) }

    private suspend fun processFile(uri: Uri): Boolean {
        val input = try {
            DataChannel.File(ParcelFileDescriptor.AutoCloseInputStream(uri.openFd()).channel)
        } catch (e: IOException) {
            console.add("! Process error")
            Timber.e(e)
            return false
        }
        return processInput(input)
    }

    private suspend fun processUrl(url: String): Boolean {
        val input = try {
            DataChannel.Http(ServiceLocator.okhttp, url)
        } catch (e: IOException) {
            console.add("! Error: " + e.message)
            Timber.e(e)
            return false
        }
        return processInput(input)
    }

    private suspend fun processInput(input: DataChannel): Boolean {
        try {
            input.use {
                PatchFileClassifier(input).use { file ->
                    logs.add("Input type: ${file.type}, payload: ${file.entryPath ?: "<input>"}")
                    return when (file.type) {
                        PatchFileClassifier.Type.Tar -> {
                            processWholeFile(file.openStream(), "tar") { out ->
                                TarProcessor(installDir, out, console, logs)
                            }
                        }
                        PatchFileClassifier.Type.RecoveryGpt -> {
                            file.entryPath?.let { console.add("- Processing $it") }
                            processWholeFile(file.openStream(), "bin") { out ->
                                RecoveryGptProcessor(installDir, out, console, logs)
                            }
                        }
                        PatchFileClassifier.Type.PayloadBin -> extractAndProcessImage { out ->
                            console.add("- Processing as OTA package")
                            OtaPayloadExtractor(out, console, logs).extract(file.openChannel())
                        }
                        PatchFileClassifier.Type.BootZip -> extractAndProcessImage { out ->
                            console.add("- Extracting: ${file.entryPath} (${file.size} bytes)")
                            file.openStream().use { it.copyAndCloseOut(out.newOutputStream()) }
                        }
                        PatchFileClassifier.Type.RawFile -> extractAndProcessImage { out ->
                            console.add("- Copying image to cache")
                            file.openStream().use { it.copyAndCloseOut(out.newOutputStream()) }
                        }
                    }
                }
            }
        } catch (e: IOException) {
            console.add("! Process error")
            Timber.e(e)
            return false
        }
    }

    // Patch the boot image in a whole file (e.g. tar archive), and output a new file
    // with all the original contents and the patched boot image
    private suspend fun processWholeFile(
        input: InputStream,
        ext: String,
        createPatcher: (OutputStream) -> WholeFilePatcher
    ): Boolean {
        val outFile = MediaStoreUtils.getFile("$destName.$ext")
        val outStream = outFile.uri.outputStream().buffered(1024 * 1024)
        val patcher = createPatcher(outStream)

        // Process input file
        try {
            targetImage = patcher.start(input)
        } catch (e: IOException) {
            patcher.close()
            runCatching { outStream.close() }
            outFile.delete()
            if (e is TarProcessor.NoBootException)
                console.add("! No boot image found")
            console.add("! Process error")
            Timber.e(e)
            return false
        }

        // Patch file
        if (!patchBoot()) {
            patcher.close()
            runCatching { outStream.close() }
            outFile.delete()
            return false
        }

        // Output file
        try {
            val newBoot = installDir.getChildFile("new-boot.img")
            patcher.finish(newBoot)
            newBoot.delete()
        } catch (e: IOException) {
            console.add("! Failed to output to $outFile")
            outFile.delete()
            Timber.e(e)
            return false
        } finally {
            patcher.close()
            outStream.close()
        }

        return onOutputWritten(outFile)
    }

    // Extract the boot image from the input, and output the patched boot image
    private suspend fun extractAndProcessImage(extract: suspend (ExtendedFile) -> Unit): Boolean {
        // Process input file
        targetImage = installDir.getChildFile("boot.img")
        try {
            extract(targetImage)
        } catch (e: IOException) {
            console.add("! Process error")
            Timber.e(e)
            return false
        }

        // Patch file
        if (!patchBoot())
            return false

        // Output file
        val outFile = MediaStoreUtils.getFile("$destName.img")
        try {
            val newBoot = installDir.getChildFile("new-boot.img")
            outFile.uri.outputStream().use { out ->
                newBoot.newInputStream().use { it.copyAll(out, 1024 * 1024) }
            }
            newBoot.delete()
        } catch (e: IOException) {
            console.add("! Failed to output to $outFile")
            outFile.delete()
            Timber.e(e)
            return false
        }

        return onOutputWritten(outFile)
    }

    private fun onOutputWritten(outFile: MediaStoreUtils.UriFile): Boolean {
        console.add("")
        console.add("****************************")
        console.add(" Output file is written to ")
        console.add(" $outFile ")
        console.add("****************************")

        // Fix up binaries
        targetImage.delete()
        "cp_readlink $installDir".sh()

        return true
    }

    private fun patchBoot(): Boolean {
        val newBoot = installDir.getChildFile("new-boot.img")
        if (!useRootFs) {
            // Create output files before hand
            newBoot.createNewFile()
            File(installDir, "stock_boot.img").createNewFile()
        }

        val cmds = arrayOf(
            "cd $installDir",
            "KEEPFORCEENCRYPT=${Config.keepEnc} " +
            "KEEPVERITY=${Config.keepVerity} " +
            "PATCHVBMETAFLAG=${Info.patchBootVbmeta} " +
            "RECOVERYMODE=${Config.recovery} " +
            "LEGACYSAR=${Info.legacySAR} " +
            "sh boot_patch.sh $targetImage")
        val isSuccess = cmds.sh().isSuccess

        shell.newJob().add("./magiskboot cleanup", "cd /").exec()

        return isSuccess
    }

    private fun flashBoot() = "direct_install $installDir $targetImage".sh().isSuccess

    private fun postOTA(): Boolean {
        "post_ota".sh()

        console.add("*************************************************************")
        console.add(" Next reboot will boot to second slot!")
        console.add(" Go back to System Updates and press Restart to complete OTA")
        console.add("*************************************************************")
        return true
    }

    private fun String.sh() = shell.newJob().add(this).to(console, logs).exec()
    private fun Array<String>.sh() = shell.newJob().add(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(shell, this)

    protected suspend fun patchFile(file: Uri) = extractFiles() && processFile(file)

    protected suspend fun patchFile(url: String) = extractFiles() && processUrl(url)

    protected suspend fun direct() = findImage() && extractFiles() && patchBoot() && flashBoot()

    protected suspend fun secondSlot() =
        findSecondary() && extractFiles() && patchBoot() && flashBoot() && postOTA()

    protected suspend fun fixEnv() = extractFiles() && "fix_env $installDir".sh().isSuccess

    protected fun restore() = findImage() && "restore_imgs $targetImage".sh().isSuccess

    protected fun uninstall() = "run_uninstaller $AppApkPath".sh().isSuccess

    @WorkerThread
    protected abstract suspend fun operations(): Boolean

    open suspend fun exec(): Boolean {
        if (haveActiveSession.getAndSet(true))
            return false

        val result = withContext(Dispatchers.IO) { operations() }
        haveActiveSession.set(false)
        if (result)
            return true

        // Not every operation initializes installDir
        if (::installDir.isInitialized)
            Shell.cmd("rm -rf $installDir").submit()
        return false
    }

    companion object {
        private var haveActiveSession = AtomicBoolean(false)
    }
}

abstract class ConsoleInstaller(
    console: MutableList<String>,
    logs: MutableList<String>
) : MagiskInstallImpl(console, logs) {
    override suspend fun exec(): Boolean {
        val success = super.exec()
        if (success) {
            console.add("- All done!")
        } else {
            console.add("! Installation failed")
        }
        return success
    }
}

abstract class CallBackInstaller : MagiskInstallImpl(DummyList, DummyList) {
    suspend fun exec(callback: (Boolean) -> Unit): Boolean {
        val success = exec()
        callback(success)
        return success
    }
}

class MagiskInstaller {

    class Patch(
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = patchFile(uri)
    }

    class Download(
        private val url: String,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = patchFile(url)
    }

    class SecondSlot(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = secondSlot()
    }

    class Direct(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = direct()
    }

    class Emulator(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = fixEnv()
    }

    class Uninstall(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = uninstall()

        override suspend fun exec(): Boolean {
            val success = super.exec()
            if (success) {
                UiThreadHandler.handler.postDelayed(3000) {
                    Shell.cmd("pm uninstall ${context.packageName}").exec()
                }
            }
            return success
        }
    }

    class Restore : CallBackInstaller() {
        override suspend fun operations() = restore()
    }

    class FixEnv : CallBackInstaller() {
        override suspend fun operations() = fixEnv()
    }
}
