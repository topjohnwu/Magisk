package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import android.system.Os
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.*
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.ktx.reboot
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.ktx.withStreams
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.core.utils.RootUtils
import com.topjohnwu.magisk.signing.SignBoot
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
                }
            } else {
                val info = context.applicationInfo
                var libs = File(info.nativeLibraryDir).listFiles { _, name ->
                    name.startsWith("lib") && name.endsWith(".so")
                } ?: emptyArray()

                // Also symlink magisk32 on non 64-bit only 64-bit devices
                val lib32 = info.javaClass.getDeclaredField("secondaryNativeLibraryDir").get(info) as String?
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

    private fun InputStream.cleanPump(out: OutputStream) = withStreams(this, out) { src, _ ->
        src.copyTo(out)
    }

    private fun newTarEntry(name: String, size: Long): TarEntry {
        console.add("-- Writing: $name")
        return TarEntry(TarHeader.createHeader(name, size, 0, false, 420 /* 0644 */))
    }

    @Throws(IOException::class)
    private fun processTar(input: InputStream, output: OutputStream): OutputStream {
        console.add("- Processing tar file")
        val tarOut = TarOutputStream(output)
        TarInputStream(input).use { tarIn ->
            lateinit var entry: TarEntry

            fun decompressedStream(): InputStream {
                val src = if (entry.name.endsWith(".lz4")) LZ4FrameInputStream(tarIn) else tarIn
                return object : FilterInputStream(src) {
                    override fun available() = 0  /* Workaround bug in LZ4FrameInputStream */
                    override fun close() { /* Never close src stream */ }
                }
            }

            while (tarIn.nextEntry?.let { entry = it } != null) {
                if (entry.name.startsWith("boot.img") ||
                    entry.name.startsWith("init_boot.img") ||
                    (Config.recovery && entry.name.contains("recovery.img"))) {
                    val name = entry.name.replace(".lz4", "")
                    console.add("-- Extracting: $name")

                    val extract = installDir.getChildFile(name)
                    decompressedStream().cleanPump(extract.newOutputStream())
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
                } else {
                    console.add("-- Copying: ${entry.name}")
                    tarOut.putNextEntry(entry)
                    tarIn.copyTo(tarOut, bufferSize = 1024 * 1024)
                }
            }
        }
        val boot = installDir.getChildFile("boot.img")
        val initBoot = installDir.getChildFile("init_boot.img")
        val recovery = installDir.getChildFile("recovery.img")
        if (Config.recovery && recovery.exists() && boot.exists()) {
            // Install to recovery
            srcBoot = recovery
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
            boot.newInputStream().use {
                tarOut.putNextEntry(newTarEntry("boot.img", boot.length()))
                it.copyTo(tarOut)
            }
            boot.delete()
        } else {
            srcBoot = when {
                initBoot.exists() -> initBoot
                boot.exists() -> boot
                else -> {
                    console.add("! No boot image found")
                    throw IOException()
                }
            }
        }
        return tarOut
    }

    private fun handleFile(uri: Uri): Boolean {
        val outStream: OutputStream
        var outFile: MediaStoreUtils.UriFile? = null

        // Process input file
        try {
            uri.inputStream().buffered().use { src ->
                src.mark(500)
                val magic = ByteArray(5)
                if (src.skip(257) != 257L || src.read(magic) != magic.size) {
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

                outStream = if (magic.contentEquals("ustar".toByteArray())) {
                    // tar file
                    outFile = MediaStoreUtils.getFile("$filename.tar", true)
                    processTar(src, outFile!!.uri.outputStream())
                } else {
                    // raw image
                    srcBoot = installDir.getChildFile("boot.img")
                    console.add("- Copying image to cache")
                    src.cleanPump(srcBoot.newOutputStream())
                    outFile = MediaStoreUtils.getFile("$filename.img", true)
                    outFile!!.uri.outputStream()
                }
            }
        } catch (e: IOException) {
            console.add("! Process error")
            outFile?.delete()
            Timber.e(e)
            return false
        }

        // Patch file
        if (!patchBoot()) {
            outFile!!.delete()
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
            newBoot.newInputStream().cleanPump(outStream)
            newBoot.delete()

            console.add("")
            console.add("****************************")
            console.add(" Output file is written to ")
            console.add(" $outFile ")
            console.add("****************************")
        } catch (e: IOException) {
            console.add("! Failed to output to $outFile")
            outFile!!.delete()
            Timber.e(e)
            return false
        }

        // Fix up binaries
        srcBoot.delete()
        "cp_readlink $installDir".sh()

        return true
    }

    private fun patchBoot(): Boolean {
        var isSigned = false
        if (!srcBoot.isCharacter) {
            try {
                srcBoot.newInputStream().use {
                    if (SignBoot.verifySignature(it, null)) {
                        isSigned = true
                        console.add("- Boot image is signed with AVB 1.0")
                    }
                }
            } catch (e: IOException) {
                console.add("! Unable to check signature")
                Timber.e(e)
                return false
            }
        }

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
            "PATCHVBMETAFLAG=${Config.patchVbmeta} " +
            "RECOVERYMODE=${Config.recovery} " +
            "SYSTEM_ROOT=${Info.isSAR} " +
            "sh boot_patch.sh $srcBoot")

        if (!cmds.sh().isSuccess)
            return false

        val job = shell.newJob().add("./magiskboot cleanup", "cd /")

        if (isSigned) {
            console.add("- Signing boot image with verity keys")
            val signed = File.createTempFile("signed", ".img", context.cacheDir)
            try {
                val src = newBoot.newInputStream().buffered()
                val out = signed.outputStream().buffered()
                withStreams(src, out) { _, _ ->
                    SignBoot.doSignature(null, null, src, out, "/boot")
                }
            } catch (e: IOException) {
                console.add("! Unable to sign image")
                Timber.e(e)
                return false
            }
            job.add("cat $signed > $newBoot", "rm -f $signed")
        }
        job.exec()
        return true
    }

    private fun flashBoot() = "direct_install $installDir $srcBoot".sh().isSuccess

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

    private fun String.sh() = shell.newJob().add(this).to(console, logs).exec()
    private fun Array<String>.sh() = shell.newJob().add(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(shell, this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(shell, *this)

    protected fun patchFile(file: Uri) = extractFiles() && handleFile(file)

    protected fun direct() = findImage() && extractFiles() && patchBoot() && flashBoot()

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
