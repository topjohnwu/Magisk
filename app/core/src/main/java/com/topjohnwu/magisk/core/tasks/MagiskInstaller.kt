package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import android.os.ParcelFileDescriptor
import android.os.Process
import android.system.Os
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.core.AppApkPath
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.copyAll
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.magisk.core.utils.DummyList
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.openFd
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.ProgressInputStream
import com.topjohnwu.magisk.core.utils.ProgressOutputStream
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.core.utils.UsbBlockDevice
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
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

            // Extract scripts
            for (script in listOf("util_functions.sh", "boot_patch.sh", "addon.d.sh")) {
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

    private suspend fun processFile(uri: Uri, usbDeviceName: String? = null): Boolean {
        val input = try {
            if (uri.scheme.equals("https", ignoreCase = true) || uri.scheme.equals("http", ignoreCase = true)) {
                DataChannel.Http(ServiceLocator.okhttp, uri.toString())
            } else {
                DataChannel.File(ParcelFileDescriptor.AutoCloseInputStream(uri.openFd()).channel)
            }
        } catch (e: IOException) {
            console.add("! Error: " + (e.message ?: "Process error"))
            Timber.e(e)
            return false
        }
        return processInput(input, usbDeviceName)
    }

    private class SpeedTracker {
        var bytes = 0L
            private set
        private var prevBytes = 0L
        private var prevTime = 0L
        private var smoothedSpeed = -1f

        val mb get() = bytes.toFloat() / 1048576

        fun speed(now: Long): Float {
            if (prevTime == 0L || now - prevTime > 2000) {
                return 0f
            }
            return maxOf(0f, smoothedSpeed)
        }

        fun update(newBytes: Long): Boolean {
            if (newBytes == bytes) return false
            bytes = newBytes
            val now = System.currentTimeMillis()
            val dt = now - prevTime
            if (prevTime == 0L || dt <= 0) {
                prevBytes = newBytes
                prevTime = now
                return false
            }
            val instantSpeed = (newBytes - prevBytes).toFloat() * 1000 / dt / 1048576
            smoothedSpeed = if (smoothedSpeed < 0f) {
                instantSpeed
            } else {
                0.3f * instantSpeed + 0.7f * smoothedSpeed
            }
            prevBytes = newBytes
            prevTime = now
            return true
        }

        fun reset() {
            prevBytes = bytes
            prevTime = 0L
            smoothedSpeed = -1f
        }
    }

    private suspend fun processInput(input: DataChannel, usbDeviceName: String? = null): Boolean {
        try {
            input.use {
                PatchFileClassifier(input).use { file ->
                    logs.add("Input type: ${file.type}, payload: ${file.entryPath ?: "<input>"}")
                    if (usbDeviceName != null && file.type != PatchFileClassifier.Type.RecoveryGpt) {
                        console.add("! Selected file is not a recovery disk image")
                        return false
                    }
                    return when (file.type) {
                        PatchFileClassifier.Type.Tar -> {
                            processWholeFile(file, "tar") { out ->
                                TarProcessor(installDir, out, console, logs)
                            }
                        }
                        PatchFileClassifier.Type.RecoveryGpt -> {
                            file.entryPath?.let { console.add("- Processing $it") }
                            if (usbDeviceName != null) {
                                processUsbRecovery(file, usbDeviceName)
                            } else {
                                processWholeFile(file, "bin") { out ->
                                    RecoveryGptProcessor(installDir, out, console, logs)
                                }
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

    private suspend fun runWholeFilePatcher(
        file: PatchFileClassifier,
        rawOut: OutputStream,
        outputLabel: String,
        createPatcher: (OutputStream) -> WholeFilePatcher
    ): Boolean {
        val max = file.rawSize
        val total = max.toFloat() / 1048576
        val readTracker = SpeedTracker()
        val writeTracker = SpeedTracker()

        fun emitProgress() {
            val now = System.currentTimeMillis()
            val readSpeed = readTracker.speed(now)
            val writeSpeed = writeTracker.speed(now)
            val readInfo = if (max > 0) {
                val pct = readTracker.bytes * 100 / max
                "%.2f / %.2f MB (%d%%) - %.2f MB/s".format(readTracker.mb, total, pct, readSpeed)
            } else {
                "%.2f MB / ?? - %.2f MB/s".format(readTracker.mb, readSpeed)
            }
            val writeInfo = "%.2f MB - %.2f MB/s".format(writeTracker.mb, writeSpeed)
            console.add("\rRead : $readInfo\nWrite: $writeInfo")
        }

        val outStream = ProgressOutputStream(rawOut) { bytes ->
            if (writeTracker.update(bytes)) emitProgress()
        }
        val patcher = createPatcher(outStream)

        // Process input file
        try {
            val input = file.openStream { raw ->
                ProgressInputStream(raw) { bytes ->
                    if (readTracker.update(bytes)) emitProgress()
                }
            }
            targetImage = patcher.start(input)
        } catch (e: IOException) {
            patcher.close()
            runCatching { outStream.close() }
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
            return false
        }

        readTracker.reset()
        writeTracker.reset()

        // Output file
        try {
            val newBoot = installDir.getChildFile("new-boot.img")
            patcher.finish(newBoot)
            newBoot.delete()
            outStream.close()
        } catch (e: IOException) {
            patcher.close()
            runCatching { outStream.close() }
            console.add("! Failed to output to $outputLabel")
            Timber.e(e)
            return false
        } finally {
            patcher.close()
            runCatching { outStream.close() }
        }

        return true
    }

    // Patch the boot image in a whole file (e.g. tar archive), and output a new file
    // with all the original contents and the patched boot image
    private suspend fun processWholeFile(
        file: PatchFileClassifier,
        ext: String,
        createPatcher: (OutputStream) -> WholeFilePatcher
    ): Boolean {
        val outFile = MediaStoreUtils.getFile("$destName.$ext")
        val outStream = outFile.uri.outputStream().buffered(1024 * 1024)
        if (!runWholeFilePatcher(file, outStream, outFile.toString(), createPatcher)) {
            outFile.delete()
            return false
        }
        return onOutputWritten(outFile)
    }

    private suspend fun processUsbRecovery(file: PatchFileClassifier, usbDeviceName: String): Boolean {
        val outStream = try {
            UsbBlockDevice.open(context, usbDeviceName, file.size, console, logs)
        } catch (e: IOException) {
            console.add("! " + (e.message ?: "Failed to open USB drive"))
            Timber.e(e)
            return false
        }
        if (!runWholeFilePatcher(file, outStream, usbDeviceName) { out ->
            RecoveryGptProcessor(installDir, out, console, logs)
        }) {
            return false
        }

        console.add("")
        console.add("****************************")
        console.add(" Recovery image flashed to USB ")
        console.add("****************************")

        targetImage.delete()
        "cp_readlink $installDir".sh()
        return true
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

    protected suspend fun patchFile(file: Uri, usbDeviceName: String? = null) =
        extractFiles() && processFile(file, usbDeviceName)

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
        private val usbDeviceName: String? = null,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : ConsoleInstaller(console, logs) {
        override suspend fun operations() = patchFile(uri, usbDeviceName)
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
