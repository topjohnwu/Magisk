package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import android.system.ErrnoException
import android.system.Os
import android.system.OsConstants
import android.system.OsConstants.O_WRONLY
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.AppApkPath
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.copyAndClose
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.NOPList
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import net.jpountz.lz4.LZ4FrameInputStream
import org.kamranzafar.jtar.TarEntry
import org.kamranzafar.jtar.TarHeader
import org.kamranzafar.jtar.TarInputStream
import org.kamranzafar.jtar.TarOutputStream
import timber.log.Timber
import java.io.*
import java.nio.ByteBuffer
import java.security.SecureRandom
import java.util.*
import java.util.concurrent.atomic.AtomicBoolean
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipInputStream

abstract class MagiskInstallImpl protected constructor(
    protected val console: MutableList<String> = NOPList.getInstance(),
    private val logs: MutableList<String> = NOPList.getInstance()
) {

    protected lateinit var installDir: ExtendedFile
    private lateinit var srcBoot: ExtendedFile

    private val shell = Shell.getShell()
    private val service get() = ServiceLocator.networkService
    protected val context get() = ServiceLocator.deContext
    private val useRootDir = shell.isRoot && Info.noDataExec

    private val rootFS get() = RootUtils.fs
    private val localFS get() = FileSystemManager.getLocal()

    private fun findImage(): Boolean {
        val bootPath = "RECOVERYMODE=${Config.recovery} find_boot_image; echo \"\$BOOTIMAGE\"".fsh()
        if (bootPath.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        srcBoot = rootFS.getFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun findSecondary(): Boolean {
        val slot = "echo \$SLOT".fsh()
        val target = if (slot == "_a") "_b" else "_a"
        console.add("- Target slot: $target")
        val bootPath = arrayOf(
            "SLOT=$target",
            "find_boot_image",
            "SLOT=$slot",
            "echo \"\$BOOTIMAGE\"").fsh()
        if (bootPath.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        srcBoot = rootFS.getFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun extractFiles(): Boolean {
        console.add("- Device platform: ${Const.CPU_ABI}")
        console.add("- Installing: ${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})")

        installDir = localFS.getFile(context.filesDir.parent, "install")
        installDir.deleteRecursively()
        installDir.mkdirs()

        try {
            // Extract binaries
            if (isRunningAsStub) {
                val zf = ZipFile(StubApk.current(context))

                // Also extract magisk32 on non 64-bit only 64-bit devices
                val is32lib = Const.CPU_ABI_32?.let {
                    { entry: ZipEntry -> entry.name == "lib/$it/libmagisk32.so" }
                } ?: { false }

                zf.entries().asSequence().filter {
                    !it.isDirectory && (it.name.startsWith("lib/${Const.CPU_ABI}/") || is32lib(it))
                }.forEach {
                    val n = it.name.substring(it.name.lastIndexOf('/') + 1)
                    val name = n.substring(3, n.length - 3)
                    val dest = File(installDir, name)
                    zf.getInputStream(it).writeTo(dest)
                    dest.setExecutable(true)
                }
                zf.close()
            } else {
                val info = context.applicationInfo
                var libs = File(info.nativeLibraryDir).listFiles { _, name ->
                    name.startsWith("lib") && name.endsWith(".so")
                } ?: emptyArray()

                // Also symlink magisk32 on non 64-bit only 64-bit devices
                val lib32 = info.javaClass.getDeclaredField("secondaryNativeLibraryDir")
                    .get(info) as String?
                if (lib32 != null) {
                    libs += File(lib32, "libmagisk32.so")
                }

                for (lib in libs) {
                    val name = lib.name.substring(3, lib.name.length - 3)
                    Os.symlink(lib.path, "$installDir/$name")
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

    private fun InputStream.copyAndCloseOut(out: OutputStream) = out.use { copyTo(it) }

    private fun newTarEntry(name: String, size: Long): TarEntry {
        console.add("-- Writing: $name")
        return TarEntry(TarHeader.createHeader(name, size, 0, false, 420 /* 0644 */))
    }

    private class NoAvailableStream(s: InputStream) : FilterInputStream(s) {
        // Make sure available is never called on the actual stream and always return 0
        // 1. Workaround bug in LZ4FrameInputStream
        // 2. Reduce max buffer size to prevent OOM
        override fun available() = 0
    }

    private class NoBootException : IOException()

    @Throws(IOException::class)
    private fun processTar(tarIn: TarInputStream, tarOut: TarOutputStream): ExtendedFile {
        console.add("- Processing tar file")
        lateinit var entry: TarEntry

        fun decompressedStream(): InputStream {
            val stream = if (entry.name.endsWith(".lz4")) LZ4FrameInputStream(tarIn) else tarIn
            return NoAvailableStream(stream)
        }

        while (tarIn.nextEntry?.let { entry = it } != null) {
            if (entry.name.startsWith("boot.img") ||
                entry.name.startsWith("init_boot.img") ||
                (Config.recovery && entry.name.contains("recovery.img"))) {
                val name = entry.name.replace(".lz4", "")
                console.add("-- Extracting: $name")

                val extract = installDir.getChildFile(name)
                decompressedStream().copyAndCloseOut(extract.newOutputStream())
            } else if (entry.name.contains("vbmeta.img")) {
                val rawData = decompressedStream().readBytes()
                // Valid vbmeta.img should be at least 256 bytes
                if (rawData.size < 256)
                    continue

                // Patch flags to AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED |
                // AVB_VBMETA_IMAGE_FLAGS_VERIFICATION_DISABLED
                console.add("-- Patching: vbmeta.img")
                ByteBuffer.wrap(rawData).putInt(120, 3)
                tarOut.putNextEntry(newTarEntry("vbmeta.img", rawData.size.toLong()))
                tarOut.write(rawData)
                // vbmeta partition exist, disable boot vbmeta patch
                Info.patchBootVbmeta = false
            } else if (entry.name.contains("userdata.img")) {
                continue
            } else {
                console.add("-- Copying: ${entry.name}")
                tarOut.putNextEntry(entry)
                tarIn.copyTo(tarOut, bufferSize = 1024 * 1024)
            }
        }

        val boot = installDir.getChildFile("boot.img")
        val initBoot = installDir.getChildFile("init_boot.img")
        val recovery = installDir.getChildFile("recovery.img")

        fun ExtendedFile.copyToTar() {
            newInputStream().use {
                tarOut.putNextEntry(newTarEntry(name, length()))
                it.copyTo(tarOut)
            }
            delete()
        }

        // Patch priority: recovery > init_boot > boot
        return when {
            recovery.exists() -> {
                if (boot.exists()) {
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
                    boot.copyToTar()
                }
                recovery
            }
            initBoot.exists() -> {
                if (boot.exists())
                    boot.copyToTar()
                initBoot
            }
            boot.exists() -> boot
            else -> throw NoBootException()
        }
    }

    @Throws(IOException::class)
    private fun processZip(zipIn: ZipInputStream): ExtendedFile {
        console.add("- Processing zip file")
        val boot = installDir.getChildFile("boot.img")
        val initBoot = installDir.getChildFile("init_boot.img")
        lateinit var entry: ZipEntry
        while (zipIn.nextEntry?.also { entry = it } != null) {
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

    private fun handleFile(uri: Uri): Boolean {
        val outStream: OutputStream
        val outFile: MediaStoreUtils.UriFile

        // Process input file
        try {
            uri.inputStream().buffered().use { src ->
                src.mark(500)
                val magic = ByteArray(4)
                val tarMagic = ByteArray(5)
                if (src.read(magic) != magic.size || src.skip(253) != 253L ||
                    src.read(tarMagic) != tarMagic.size
                ) {
                    console.add("! Invalid input file")
                    return false
                }
                src.reset()

                val alpha = "abcdefghijklmnopqrstuvwxyz"
                val alphaNum = "$alpha${alpha.uppercase(Locale.ROOT)}0123456789"
                val random = SecureRandom()
                val filename = StringBuilder("magisk_patched-${BuildConfig.VERSION_CODE}_").run {
                    for (i in 1..5) {
                        append(alphaNum[random.nextInt(alphaNum.length)])
                    }
                    toString()
                }

                srcBoot = if (tarMagic.contentEquals("ustar".toByteArray())) {
                    // tar file
                    outFile = MediaStoreUtils.getFile("$filename.tar", true)
                    outStream = TarOutputStream(outFile.uri.outputStream())

                    try {
                        processTar(TarInputStream(src), outStream)
                    } catch (e: IOException) {
                        outStream.close()
                        outFile.delete()
                        throw e
                    }
                } else {
                    // raw image
                    outFile = MediaStoreUtils.getFile("$filename.img", true)
                    outStream = outFile.uri.outputStream()

                    try {
                        if (magic.contentEquals("CrAU".toByteArray())) {
                            processPayload(src)
                        } else if (magic.contentEquals("PK\u0003\u0004".toByteArray())) {
                            processZip(ZipInputStream(src))
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
            if (outStream is TarOutputStream) {
                val name = with(srcBoot.path) {
                    when {
                        contains("recovery") -> "recovery.img"
                        contains("init_boot") -> "init_boot.img"
                        else -> "boot.img"
                    }
                }
                outStream.putNextEntry(newTarEntry(name, newBoot.length()))
            }
            newBoot.newInputStream().copyAndClose(outStream)
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

    private fun flashBoot() = "direct_install \"$installDir\" \"$srcBoot\" \"$AppApkPath\"".sh().isSuccess

    private fun postOTA(): Boolean {
        try {
            val bootctl = File.createTempFile("bootctl", null, context.cacheDir)
            context.assets.open("bootctl").writeTo(bootctl)
            "post_ota $bootctl".sh()
        } catch (e: IOException) {
            console.add("! Unable to download bootctl")
            Timber.e(e)
            return false
        }

        console.add("***************************************")
        console.add(" Next reboot will boot to second slot!")
        console.add("***************************************")
        return true
    }

    private fun Array<String>.eq() = shell.newJob().add(*this).to(console, logs).enqueue()
    private fun String.sh() = shell.newJob().add(this).to(console, logs).exec()
    private fun Array<String>.sh() = shell.newJob().add(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(shell, this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(shell, *this)

    protected fun patchFile(file: Uri) = extractFiles() && handleFile(file)

    protected fun direct() = findImage() && extractFiles() && patchBoot() && flashBoot()

    protected fun direct_system() = extractFiles() && "xdirect_install_system \"$installDir\" \"dummy\" \"$AppApkPath\"".sh().isSuccess

    protected fun secondSlot() =
        findSecondary() && extractFiles() && patchBoot() && flashBoot() && postOTA()

    protected fun fixEnv() = extractFiles() && "fix_env $installDir".sh().isSuccess

    protected fun uninstall() = "run_uninstaller $AppApkPath".sh().isSuccess

    @WorkerThread
    protected abstract suspend fun operations(): Boolean

    open suspend fun exec(): Boolean {
        if (haveActiveSession.getAndSet(true))
            return false
        val result = withContext(Dispatchers.IO) { operations() }
        haveActiveSession.set(false)
        return result
    }

    companion object {
        private var haveActiveSession = AtomicBoolean(false)
    }
}

abstract class MagiskInstaller(
    console: MutableList<String>,
    logs: MutableList<String>
) : MagiskInstallImpl(console, logs) {

    override suspend fun exec(): Boolean {
        val success = super.exec()
        if (success) {
            console.add("- All done!")
        } else {
            Shell.cmd("rm -rf $installDir").submit()
            console.add("! Installation failed")
        }
        return success
    }

    class Patch(
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = patchFile(uri)
    }

    class SecondSlot(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = secondSlot()
    }

    class Direct(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = direct()
    }

    class Direct_system(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = direct_system()
    }

    class Emulator(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = fixEnv()
    }

    class Uninstall(
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstallImpl(console, logs) {
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

    class FixEnv(private val callback: () -> Unit) : MagiskInstallImpl() {
        override suspend fun operations() = fixEnv()

        override suspend fun exec(): Boolean {
            val success = super.exec()
            callback()
            context.toast(
                if (success) R.string.reboot_delay_toast else R.string.setup_fail,
                Toast.LENGTH_LONG
            )
            if (success)
                UiThreadHandler.handler.postDelayed(5000) { reboot() }
            return success
        }
    }
}
