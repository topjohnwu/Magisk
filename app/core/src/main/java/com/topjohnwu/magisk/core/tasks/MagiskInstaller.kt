package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import android.os.Process
import android.system.ErrnoException
import android.system.Os
import android.system.OsConstants
import android.system.OsConstants.O_WRONLY
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
import com.topjohnwu.magisk.core.utils.DummyList
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.apache.commons.compress.archivers.tar.TarArchiveEntry
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipArchiveInputStream
import org.apache.commons.compress.archivers.zip.ZipFile
import org.apache.commons.compress.compressors.lz4.FramedLZ4CompressorInputStream
import timber.log.Timber
import java.io.File
import java.io.FilterInputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.io.PushbackInputStream
import java.nio.ByteBuffer
import java.security.SecureRandom
import java.util.Locale
import java.util.concurrent.atomic.AtomicBoolean

abstract class MagiskInstallImpl protected constructor(
    protected val console: MutableList<String>,
    private val logs: MutableList<String>
) {

    private lateinit var installDir: ExtendedFile
    private lateinit var srcBoot: ExtendedFile

    private val shell = Shell.getShell()
    private val useRootDir = shell.isRoot && Info.noDataExec
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
        val bootPath = (
            "(RECOVERYMODE=${Config.recovery} " +
            "SLOT=$slot find_boot_image; " +
            "echo \$BOOTIMAGE)").fsh()
        if (bootPath.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        srcBoot = rootFS.getFile(bootPath)
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
            // Extract chromeos tools
            File(installDir, "chromeos").mkdir()
            for (file in listOf("futility", "kernel_data_key.vbprivk", "kernel.keyblock")) {
                val name = "chromeos/$file"
                val dest = File(installDir, name)
                context.assets.open(name).writeTo(dest)
            }
        } catch (e: Exception) {
            console.add("! Unable to extract files")
            Timber.e(e)
            return false
        }

        if (useRootDir) {
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

    private class NoAvailableStream(s: InputStream) : FilterInputStream(s) {
        // Make sure available is never called on the actual stream and always return 0
        // to reduce max buffer size and avoid OOM
        override fun available() = 0
    }

    private class NoBootException : IOException()

    inner class BootItem(private val entry: TarArchiveEntry) {
        val name = entry.name.replace(".lz4", "")
        var file = installDir.getChildFile(name)

        suspend fun copyTo(tarOut: TarArchiveOutputStream) {
            entry.name = name
            entry.size = file.length()
            file.newInputStream().use {
                console.add("-- Writing   : $name")
                tarOut.putArchiveEntry(entry)
                it.copyAll(tarOut)
                tarOut.closeArchiveEntry()
            }
        }
    }

    @Throws(IOException::class)
    private suspend fun processTar(
        tarIn: TarArchiveInputStream,
        tarOut: TarArchiveOutputStream
    ): BootItem {
        console.add("- Processing tar file")
        var entry: TarArchiveEntry? = tarIn.nextEntry

        fun decompressedStream(): InputStream {
            val stream = if (tarIn.currentEntry.name.endsWith(".lz4"))
                FramedLZ4CompressorInputStream(tarIn, true) else tarIn
            return NoAvailableStream(stream)
        }

        var boot: BootItem? = null
        var initBoot: BootItem? = null
        var recovery: BootItem? = null

        while (entry != null) {
            val bootItem: BootItem?
            if (entry.name.startsWith("boot.img")) {
                bootItem = BootItem(entry)
                boot = bootItem
            } else if (entry.name.startsWith("init_boot.img")) {
                bootItem = BootItem(entry)
                initBoot = bootItem
            } else if (Config.recovery && entry.name.contains("recovery.img")) {
                bootItem = BootItem(entry)
                recovery = bootItem
            } else {
                bootItem = null
            }

            if (bootItem != null) {
                console.add("-- Extracting: ${bootItem.name}")
                decompressedStream().copyAndCloseOut(bootItem.file.newOutputStream())
            } else if (entry.name.contains("vbmeta.img")) {
                val rawData = decompressedStream().readBytes()
                // Valid vbmeta.img should be at least 256 bytes
                if (rawData.size < 256)
                    continue

                // vbmeta partition exist, disable boot vbmeta patch
                Info.patchBootVbmeta = false

                val name = entry.name.replace(".lz4", "")
                console.add("-- Patching  : $name")

                // Patch flags to AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED |
                // AVB_VBMETA_IMAGE_FLAGS_VERIFICATION_DISABLED
                ByteBuffer.wrap(rawData).putInt(120, 3)

                // Fetch the next entry first before modifying current entry
                val vbmeta = entry
                entry = tarIn.nextEntry

                // Update entry with new information
                vbmeta.name = name
                vbmeta.size = rawData.size.toLong()

                // Write output
                tarOut.putArchiveEntry(vbmeta)
                tarOut.write(rawData)
                tarOut.closeArchiveEntry()
                continue
            } else if (entry.name.contains("userdata.img")) {
                console.add("-- Skipping  : ${entry.name}")
            } else {
                console.add("-- Copying   : ${entry.name}")
                tarOut.putArchiveEntry(entry)
                tarIn.copyAll(tarOut)
                tarOut.closeArchiveEntry()
            }
            entry = tarIn.nextEntry ?: break
        }

        // Patch priority: recovery > init_boot > boot
        return when {
            recovery != null -> {
                if (boot != null) {
                    // Repack boot image to prevent auto restore
                    arrayOf(
                        "cd $installDir",
                        "chmod -R 755 .",
                        "./magiskboot unpack boot.img",
                        "./magiskboot repack boot.img",
                        "cat new-boot.img > boot.img",
                        "./magiskboot cleanup",
                        "rm -f new-boot.img",
                        "cd /").sh()
                    boot.copyTo(tarOut)
                }
                recovery
            }
            initBoot != null -> {
                boot?.copyTo(tarOut)
                initBoot
            }
            boot != null -> boot
            else -> throw NoBootException()
        }
    }

    @Throws(IOException::class)
    private suspend fun processZip(zipIn: ZipArchiveInputStream): ExtendedFile {
        console.add("- Processing zip file")
        val boot = installDir.getChildFile("boot.img")
        val initBoot = installDir.getChildFile("init_boot.img")
        var entry: ZipArchiveEntry
        while (true) {
            entry = zipIn.nextEntry ?: break
            if (entry.isDirectory) continue
            when (entry.name.substringAfterLast('/')) {
                "payload.bin" -> {
                    try {
                        return processPayload(zipIn)
                    } catch (e: IOException) {
                        // No boot image in payload.bin, continue to find boot images
                    }
                }
                "init_boot.img" -> {
                    console.add("- Extracting init_boot.img")
                    zipIn.copyAndCloseOut(initBoot.newOutputStream())
                    return initBoot
                }
                "boot.img" -> {
                    console.add("- Extracting boot.img")
                    zipIn.copyAndCloseOut(boot.newOutputStream())
                    // Don't return here since there might be an init_boot.img
                }
            }
        }
        if (boot.exists()) {
            return boot
        } else {
            throw NoBootException()
        }
    }

    @Throws(IOException::class)
    private fun processPayload(input: InputStream): ExtendedFile {
        var fifo: File? = null
        try {
            console.add("- Processing payload.bin")
            fifo = File.createTempFile("payload-fifo-", null, installDir)
            fifo.delete()
            Os.mkfifo(fifo.path, 420 /* 0644 */)

            // Enqueue the shell command first, or the subsequent FIFO open will block
            val future = arrayOf(
                "cd $installDir",
                "./magiskboot extract $fifo",
                "cd /"
            ).eq()

            val fd = Os.open(fifo.path, O_WRONLY, 0)
            try {
                val bufSize = 1024 * 1024
                val buf = ByteBuffer.allocate(bufSize)
                buf.position(input.read(buf.array()).coerceAtLeast(0)).flip()
                while (buf.hasRemaining()) {
                    try {
                        Os.write(fd, buf)
                    } catch (e: ErrnoException) {
                        if (e.errno != OsConstants.EPIPE)
                            throw e
                        // If SIGPIPE, then the other side is closed, we're done
                        break
                    }
                    if (!buf.hasRemaining()) {
                        buf.limit(bufSize)
                        buf.position(input.read(buf.array()).coerceAtLeast(0)).flip()
                    }
                }
            } finally {
                Os.close(fd)
            }

            val success = try { future.get().isSuccess } catch (e: Exception) { false }
            if (!success) {
                console.add("! Error while extracting payload.bin")
                throw IOException()
            }
            val boot = installDir.getChildFile("boot.img")
            val initBoot = installDir.getChildFile("init_boot.img")
            return when {
                initBoot.exists() -> {
                    console.add("-- Extract init_boot.img")
                    initBoot
                }
                boot.exists() -> {
                    console.add("-- Extract boot.img")
                    boot
                }
                else -> {
                    throw NoBootException()
                }
            }
        } catch (e: ErrnoException) {
            throw IOException(e)
        } finally {
            fifo?.delete()
        }
    }

    private suspend fun processFile(uri: Uri): Boolean {
        val outStream: OutputStream
        val outFile: MediaStoreUtils.UriFile
        var bootItem: BootItem? = null

        // Process input file
        try {
            PushbackInputStream(uri.inputStream().buffered(1024 * 1024), 512).use { src ->
                val head = ByteArray(512)
                if (src.read(head) != head.size) {
                    console.add("! Invalid input file")
                    return false
                }
                src.unread(head)

                val magic = head.copyOf(4)
                val tarMagic = head.copyOfRange(257, 262)

                srcBoot = if (tarMagic.contentEquals("ustar".toByteArray())) {
                    // tar file
                    outFile = MediaStoreUtils.getFile("$destName.tar")
                    val os = outFile.uri.outputStream().buffered(1024 * 1024)
                    outStream = TarArchiveOutputStream(os).also {
                        it.setBigNumberMode(TarArchiveOutputStream.BIGNUMBER_STAR)
                        it.setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU)
                    }

                    try {
                        bootItem = processTar(TarArchiveInputStream(src), outStream)
                        bootItem.file
                    } catch (e: IOException) {
                        outStream.close()
                        outFile.delete()
                        throw e
                    }
                } else {
                    // raw image
                    outFile = MediaStoreUtils.getFile("$destName.img")
                    outStream = outFile.uri.outputStream()

                    try {
                        if (magic.contentEquals("CrAU".toByteArray())) {
                            processPayload(src)
                        } else if (magic.contentEquals("PK\u0003\u0004".toByteArray())) {
                            processZip(ZipArchiveInputStream(src))
                        } else {
                            console.add("- Copying image to cache")
                            installDir.getChildFile("boot.img").also {
                                src.copyAndCloseOut(it.newOutputStream())
                            }
                        }
                    } catch (e: IOException) {
                        outStream.close()
                        outFile.delete()
                        throw e
                    }
                }
            }
        } catch (e: IOException) {
            if (e is NoBootException)
                console.add("! No boot image found")
            console.add("! Process error")
            Timber.e(e)
            return false
        }

        // Patch file
        if (!patchBoot()) {
            outFile.delete()
            return false
        }

        // Output file
        try {
            val newBoot = installDir.getChildFile("new-boot.img")
            if (bootItem != null) {
                bootItem.file = newBoot
                bootItem.copyTo(outStream as TarArchiveOutputStream)
            } else {
                newBoot.newInputStream().use { it.copyAll(outStream, 1024 * 1024) }
            }
            newBoot.delete()

            console.add("")
            console.add("****************************")
            console.add(" Output file is written to ")
            console.add(" $outFile ")
            console.add("****************************")
        } catch (e: IOException) {
            console.add("! Failed to output to $outFile")
            outFile.delete()
            Timber.e(e)
            return false
        } finally {
            outStream.close()
        }

        // Fix up binaries
        srcBoot.delete()
        "cp_readlink $installDir".sh()

        return true
    }

    private fun patchBoot(): Boolean {
        val newBoot = installDir.getChildFile("new-boot.img")
        if (!useRootDir) {
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
            "sh boot_patch.sh $srcBoot")
        val isSuccess = cmds.sh().isSuccess

        shell.newJob().add("./magiskboot cleanup", "cd /").exec()

        return isSuccess
    }

    private fun flashBoot() = "direct_install $installDir $srcBoot".sh().isSuccess

    private suspend fun postOTA(): Boolean {
        try {
            val bootctl = File.createTempFile("bootctl", null, context.cacheDir)
            context.assets.open("bootctl").writeTo(bootctl)
            "post_ota $bootctl".sh()
        } catch (e: IOException) {
            console.add("! Unable to download bootctl")
            Timber.e(e)
            return false
        }

        console.add("*************************************************************")
        console.add(" Next reboot will boot to second slot!")
        console.add(" Go back to System Updates and press Restart to complete OTA")
        console.add("*************************************************************")
        return true
    }

    private fun Array<String>.eq() = shell.newJob().add(*this).to(console, logs).enqueue()
    private fun String.sh() = shell.newJob().add(this).to(console, logs).exec()
    private fun Array<String>.sh() = shell.newJob().add(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(shell, this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(shell, *this)

    protected suspend fun patchFile(file: Uri) = extractFiles() && processFile(file)

    protected suspend fun direct() = findImage() && extractFiles() && patchBoot() && flashBoot()

    protected suspend fun secondSlot() =
        findSecondary() && extractFiles() && patchBoot() && flashBoot() && postOTA()

    protected suspend fun fixEnv() = extractFiles() && "fix_env $installDir".sh().isSuccess

    protected fun restore() = findImage() && "restore_imgs $srcBoot".sh().isSuccess

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
