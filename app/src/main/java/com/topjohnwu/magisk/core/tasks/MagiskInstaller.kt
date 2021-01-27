package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.net.Uri
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.ktx.reboot
import com.topjohnwu.magisk.ktx.symlink
import com.topjohnwu.magisk.ktx.withStreams
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.signing.SignBoot
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.NOPList
import com.topjohnwu.superuser.internal.UiThreadHandler
import com.topjohnwu.superuser.io.SuFile
import com.topjohnwu.superuser.io.SuFileInputStream
import com.topjohnwu.superuser.io.SuFileOutputStream
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import net.jpountz.lz4.LZ4FrameInputStream
import org.kamranzafar.jtar.TarEntry
import org.kamranzafar.jtar.TarHeader
import org.kamranzafar.jtar.TarInputStream
import org.kamranzafar.jtar.TarOutputStream
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.*
import java.nio.ByteBuffer
import java.security.SecureRandom
import java.util.*
import java.util.zip.ZipFile

abstract class MagiskInstallImpl protected constructor(
    protected val console: MutableList<String> = NOPList.getInstance(),
    private val logs: MutableList<String> = NOPList.getInstance()
) : KoinComponent {

    protected lateinit var installDir: File
    private lateinit var srcBoot: File

    private var tarOut: TarOutputStream? = null
    private val service: NetworkService by inject()
    protected val context: Context by inject(Protected)

    private fun findImage(): Boolean {
        val bootPath = "find_boot_image; echo \"\$BOOTIMAGE\"".fsh()
        if (bootPath.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        srcBoot = SuFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun findSecondaryImage(): Boolean {
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
        srcBoot = SuFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun installDirFile(name: String): File {
        return if (installDir is SuFile)
            SuFile(installDir, name)
        else
            File(installDir, name)
    }

    private fun extractFiles(): Boolean {
        console.add("- Device platform: ${Const.CPU_ABI}")
        console.add("- Installing: ${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})")

        val binDir = File(context.filesDir.parent, "install")
        binDir.deleteRecursively()
        binDir.mkdirs()

        installDir = if (Shell.rootAccess()) {
            SuFile("${Const.TMPDIR}/install")
        } else {
            binDir
        }

        try {
            // Extract binaries
            if (isRunningAsStub) {
                val zf = ZipFile(DynAPK.current(context))
                zf.entries().asSequence().filter {
                    !it.isDirectory && it.name.startsWith("lib/${Const.CPU_ABI_32}/")
                }.forEach {
                    val n = it.name.substring(it.name.lastIndexOf('/') + 1)
                    val name = n.substring(3, n.length - 3)
                    val dest = File(binDir, name)
                    zf.getInputStream(it).writeTo(dest)
                }
            } else {
                val libs = Const.NATIVE_LIB_DIR.listFiles { _, name ->
                    name.startsWith("lib") && name.endsWith(".so")
                } ?: emptyArray()
                for (lib in libs) {
                    val name = lib.name.substring(3, lib.name.length - 3)
                    val bin = File(binDir, name)
                    symlink(lib.path, bin.path)
                }
            }

            // Extract scripts
            for (script in listOf("util_functions.sh", "boot_patch.sh", "addon.d.sh")) {
                val dest = File(binDir, script)
                context.assets.open(script).writeTo(dest)
            }
            // Extract chromeos tools
            File(binDir, "chromeos").mkdir()
            for (file in listOf("futility", "kernel_data_key.vbprivk", "kernel.keyblock")) {
                val name = "chromeos/$file"
                val dest = File(binDir, name)
                context.assets.open(name).writeTo(dest)
            }
        } catch (e: Exception) {
            console.add("! Unable to extract files")
            Timber.e(e)
            return false
        }

        if (installDir !== binDir) {
            arrayOf(
                "rm -rf $installDir",
                "mkdir -p $installDir",
                "cp_readlink $binDir $installDir",
                "rm -rf $binDir"
            ).sh()
        }

        return true
    }

    private fun newEntry(name: String, size: Long): TarEntry {
        console.add("-- Writing: $name")
        return TarEntry(TarHeader.createHeader(name, size, 0, false, 420 /* 0644 */))
    }

    @Throws(IOException::class)
    private fun handleTar(input: InputStream, output: OutputStream): OutputStream {
        console.add("- Processing tar file")
        val tarOut = TarOutputStream(output)
        TarInputStream(input).use { tarIn ->
            lateinit var entry: TarEntry

            fun decompressedStream() =
                if (entry.name.endsWith(".lz4")) LZ4FrameInputStream(tarIn) else tarIn

            while (tarIn.nextEntry?.let { entry = it } != null) {
                if (entry.name.contains("boot.img") ||
                    (Config.recovery && entry.name.contains("recovery.img"))) {
                    val name = entry.name.replace(".lz4", "")
                    console.add("-- Extracting: $name")

                    val extract = installDirFile(name)
                    SuFileOutputStream(extract).use { decompressedStream().copyTo(it) }
                } else if (entry.name.contains("vbmeta.img")) {
                    // DO NOT USE readBytes() DUE TO BUG IN LZ4FrameInputStream
                    val rawData = decompressedStream().run {
                        val buffer = ByteArrayOutputStream()
                        copyTo(buffer)
                        buffer.toByteArray()
                    }
                    // Valid vbmeta.img should be at least 256 bytes
                    if (rawData.size < 256)
                        continue

                    // Patch flags to AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED |
                    // AVB_VBMETA_IMAGE_FLAGS_VERIFICATION_DISABLED
                    console.add("-- Patching: vbmeta.img")
                    ByteBuffer.wrap(rawData).putInt(120, 3)
                    tarOut.putNextEntry(newEntry("vbmeta.img", rawData.size.toLong()))
                    tarOut.write(rawData)
                } else {
                    console.add("-- Copying: ${entry.name}")
                    tarOut.putNextEntry(entry)
                    tarIn.copyTo(tarOut, bufferSize = 1024 * 1024)
                }
            }
        }
        val boot = installDirFile("boot.img")
        val recovery = installDirFile("recovery.img")
        if (Config.recovery && recovery.exists() && boot.exists()) {
            // Install Magisk to recovery
            srcBoot = recovery
            // Repack boot image to prevent restore
            arrayOf(
                "./magiskboot unpack boot.img",
                "./magiskboot repack boot.img",
                "./magiskboot cleanup",
                "mv new-boot.img boot.img").sh()
            SuFileInputStream(boot).use {
                tarOut.putNextEntry(newEntry("boot.img", boot.length()))
                it.copyTo(tarOut)
            }
            boot.delete()
        } else {
            if (!boot.exists()) {
                console.add("! No boot image found")
                throw IOException()
            }
            srcBoot = boot
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
                val alphaNum = "$alpha${alpha.toUpperCase(Locale.ROOT)}0123456789"
                val random = SecureRandom()
                val filename = StringBuilder("magisk_patched_").run {
                    for (i in 1..5) {
                        append(alphaNum[random.nextInt(alphaNum.length)])
                    }
                    toString()
                }

                outStream = if (magic.contentEquals("ustar".toByteArray())) {
                    outFile = MediaStoreUtils.getFile("$filename.tar", true)
                    handleTar(src, outFile!!.uri.outputStream())
                } else {
                    // Raw image
                    srcBoot = installDirFile("boot.img")
                    console.add("- Copying image to cache")
                    SuFileOutputStream(srcBoot).use { src.copyTo(it) }
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
            val patched = installDirFile("new-boot.img")
            if (outStream is TarOutputStream) {
                val name = if (srcBoot.path.contains("recovery")) "recovery.img" else "boot.img"
                outStream.putNextEntry(newEntry(name, patched.length()))
            }
            withStreams(SuFileInputStream(patched), outStream) { src, out -> src.copyTo(out) }
            patched.delete()

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
        if (installDir is SuFile) {
            "fix_env $installDir".sh()
        } else {
            "cp_readlink $installDir".sh()
        }

        return true
    }

    private fun patchBoot(): Boolean {
        "cd $installDir".sh()

        var isSigned = false
        if (srcBoot.let { it !is SuFile || !it.isCharacter }) {
            try {
                SuFileInputStream(srcBoot).use {
                    if (SignBoot.verifySignature(it, null)) {
                        isSigned = true
                        console.add("- Boot image is signed with AVB 1.0")
                    }
                }
            } catch (e: IOException) {
                console.add("! Unable to check signature")
                return false
            }
        }

        val FLAGS =
            "KEEPFORCEENCRYPT=${Config.keepEnc} " +
            "KEEPVERITY=${Config.keepVerity} " +
            "RECOVERYMODE=${Config.recovery}"

        if (!"$FLAGS sh boot_patch.sh $srcBoot".sh().isSuccess)
            return false

        val job = Shell.sh("./magiskboot cleanup", "cd /")

        val patched = installDirFile("new-boot.img")
        if (isSigned) {
            console.add("- Signing boot image with verity keys")
            val signed = installDirFile("signed.img")
            try {
                withStreams(SuFileInputStream(patched), SuFileOutputStream(signed)) {
                    input, out -> SignBoot.doSignature(null, null, input, out, "/boot")
                }
            } catch (e: IOException) {
                console.add("! Unable to sign image")
                Timber.e(e)
                return false
            }

            job.add("mv -f $signed $patched")
        }
        job.exec()
        return true
    }

    private fun flashBoot(): Boolean {
        if (!"direct_install $installDir $srcBoot".sh().isSuccess)
            return false
        arrayOf("run_migrations", "copy_sepolicy_rules").sh()
        return true
    }

    private suspend fun postOTA(): Boolean {
        val bootctl = SuFile("/data/adb/bootctl")
        try {
            withStreams(service.fetchBootctl().byteStream(), SuFileOutputStream(bootctl)) {
                it, out -> it.copyTo(out)
            }
        } catch (e: IOException) {
            console.add("! Unable to download bootctl")
            Timber.e(e)
            return false
        }

        "post_ota ${bootctl.parent}".sh()

        console.add("***************************************")
        console.add(" Next reboot will boot to second slot!")
        console.add("***************************************")
        return true
    }

    protected fun uninstall(): Boolean {
        val apk = if (isRunningAsStub) {
            DynAPK.current(context).path
        } else {
            context.packageCodePath
        }
        return "run_uninstaller $apk".sh().isSuccess
    }

    private fun String.sh() = Shell.sh(this).to(console, logs).exec()
    private fun Array<String>.sh() = Shell.sh(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(*this)

    protected fun doPatchFile(patchFile: Uri) = extractFiles() && handleFile(patchFile)

    protected fun direct() = findImage() && extractFiles() && patchBoot() && flashBoot()

    protected suspend fun secondSlot() =
        findSecondaryImage() && extractFiles() && patchBoot() && flashBoot() && postOTA()

    protected fun fixEnv() = extractFiles() && "fix_env $installDir".sh().isSuccess

    @WorkerThread
    protected abstract suspend fun operations(): Boolean

    open suspend fun exec() = withContext(Dispatchers.IO) { operations() }
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
            if (installDir is SuFile) {
                Shell.sh("rm -rf ${Const.TMPDIR}").submit()
            } else {
                Shell.sh("rm -rf $installDir").submit()
            }
            console.add("! Installation failed")
        }
        return success
    }

    class Patch(
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(console, logs) {
        override suspend fun operations() = doPatchFile(uri)
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
                    Shell.su("pm uninstall ${context.packageName}").exec()
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
            Utils.toast(
                if (success) R.string.reboot_delay_toast else R.string.setup_fail,
                Toast.LENGTH_LONG
            )
            if (success)
                UiThreadHandler.handler.postDelayed(5000) { reboot() }
            return success
        }
    }
}
