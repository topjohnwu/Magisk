package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.net.Uri
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.*
import com.topjohnwu.magisk.core.utils.BusyBoxInit
import com.topjohnwu.magisk.core.utils.LightShellInit
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

    protected var installDir = File("xxx")
    private lateinit var srcBoot: File

    private lateinit var shell: Shell
    private var closeShell = false

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
        srcBoot = SuFile(bootPath)
        console.add("- Target image: $bootPath")
        return true
    }

    private fun extractFiles(): Boolean {
        console.add("- Device platform: ${Const.CPU_ABI}")
        console.add("- Installing: ${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})")

        installDir = File(context.filesDir.parent, "install")
        installDir.deleteRecursively()
        installDir.mkdirs()

        try {
            // Extract binaries
            if (isRunningAsStub) {
                val zf = ZipFile(DynAPK.current(context))
                zf.entries().asSequence().filter {
                    !it.isDirectory && it.name.startsWith("lib/${Const.CPU_ABI_32}/")
                }.forEach {
                    val n = it.name.substring(it.name.lastIndexOf('/') + 1)
                    val name = n.substring(3, n.length - 3)
                    val dest = File(installDir, name)
                    zf.getInputStream(it).writeTo(dest)
                }
            } else {
                val libs = Const.NATIVE_LIB_DIR.listFiles { _, name ->
                    name.startsWith("lib") && name.endsWith(".so")
                } ?: emptyArray()
                for (lib in libs) {
                    val name = lib.name.substring(3, lib.name.length - 3)
                    val bin = File(installDir, name)
                    symlink(lib.path, bin.path)
                }
            }

            // Extract scripts
            for (script in listOf("util_functions.sh", "boot_patch.sh", "addon.d.sh")) {
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

        return true
    }

    private fun newTarEntry(name: String, size: Long): TarEntry {
        console.add("-- Writing: $name")
        return TarEntry(TarHeader.createHeader(name, size, 0, false, 420 /* 0644 */))
    }

    @Throws(IOException::class)
    private fun handleTar(input: InputStream, output: OutputStream): OutputStream {
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
                if (entry.name.contains("boot.img") ||
                    (Config.recovery && entry.name.contains("recovery.img"))) {
                    val name = entry.name.replace(".lz4", "")
                    console.add("-- Extracting: $name")

                    val extract = File(installDir, name)
                    decompressedStream().writeTo(extract)
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
        val boot = File(installDir, "boot.img")
        val recovery = File(installDir, "recovery.img")
        if (Config.recovery && recovery.exists() && boot.exists()) {
            // Install to recovery
            srcBoot = recovery
            // Repack boot image to prevent auto restore
            arrayOf(
                "cd $installDir",
                "./magiskboot unpack boot.img",
                "./magiskboot repack boot.img",
                "./magiskboot cleanup",
                "mv new-boot.img boot.img",
                "cd /").sh()
            boot.inputStream().use {
                tarOut.putNextEntry(newTarEntry("boot.img", boot.length()))
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
                    srcBoot = File(installDir, "boot.img")
                    console.add("- Copying image to cache")
                    src.writeTo(srcBoot)
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
            val patched = File(installDir, "new-boot.img")
            if (outStream is TarOutputStream) {
                val name = if (srcBoot.path.contains("recovery")) "recovery.img" else "boot.img"
                outStream.putNextEntry(newTarEntry(name, patched.length()))
            }
            withStreams(patched.inputStream(), outStream) { src, out -> src.copyTo(out) }
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
        srcBoot.delete()
        "cp_readlink $installDir".sh()

        return true
    }

    private fun patchBoot(): Boolean {
        val inRootDir = shell.isRoot && Info.noDataExec

        val newBootImg: File
        if (inRootDir) {
            // Migrate everything to tmpfs to workaround Samsung bullshit
            SuFile("${Const.TMPDIR}/install").also {
                arrayOf(
                    "rm -rf $it",
                    "mkdir -p $it",
                    "cp_readlink $installDir $it",
                    "rm -rf $installDir"
                ).sh()
                installDir = it
            }
            newBootImg = SuFile(installDir, "new-boot.img")
        } else {
            newBootImg = File(installDir, "new-boot.img")
            // Create the output file before hand
            newBootImg.createNewFile()
        }

        var isSigned = false
        if (srcBoot.let { it !is SuFile || !it.isCharacter }) {
            try {
                SuFileInputStream.open(srcBoot).use {
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

        val flags =
            "KEEPFORCEENCRYPT=${Config.keepEnc} " +
            "KEEPVERITY=${Config.keepVerity} " +
            "RECOVERYMODE=${Config.recovery}"

        if (!"cd $installDir; $flags sh boot_patch.sh $srcBoot".sh().isSuccess)
            return false

        val job = shell.newJob().add("./magiskboot cleanup", "cd /")

        if (isSigned) {
            console.add("- Signing boot image with verity keys")
            val signed = File.createTempFile("signed", ".img", context.cacheDir)
            try {
                withStreams(SuFileInputStream.open(newBootImg).buffered(),
                    signed.outputStream().buffered()) { src, out ->
                    SignBoot.doSignature(null, null, src, out, "/boot")
                }
            } catch (e: IOException) {
                console.add("! Unable to sign image")
                Timber.e(e)
                return false
            }
            if (inRootDir) {
                job.add("cp -f $signed $newBootImg", "rm -f $signed")
            } else {
                signed.copyTo(newBootImg, overwrite = true)
                signed.delete()
            }
        }
        job.exec()
        return true
    }

    private fun flashBoot() = "direct_install $installDir $srcBoot".sh().isSuccess

    private suspend fun postOTA(): Boolean {
        try {
            val bootctl = File.createTempFile("bootctl", null, context.cacheDir)
            service.fetchBootctl().byteStream().writeTo(bootctl)
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

    private fun setupShell(forceNonRoot: Boolean = false): Boolean {
        shell = Shell.getShell()
        if (forceNonRoot && shell.isRoot) {
            shell = Shell.Builder.create()
                .setFlags(Shell.FLAG_NON_ROOT_SHELL)
                .setInitializers(BusyBoxInit::class.java, LightShellInit::class.java)
                .build()
            closeShell = true
        }
        return true
    }

    private fun String.sh() = shell.newJob().add(this).to(console, logs).exec()
    private fun Array<String>.sh() = shell.newJob().add(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(shell, this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(shell, *this)

    protected fun doPatchFile(patchFile: Uri) =
        setupShell(true) && extractFiles() && handleFile(patchFile)

    protected fun direct() =
        setupShell() && findImage() && extractFiles() && patchBoot() && flashBoot()

    protected suspend fun secondSlot() =
        setupShell() && findSecondary() && extractFiles() && patchBoot() && flashBoot() && postOTA()

    protected fun fixEnv() =
        setupShell() && extractFiles() && "fix_env $installDir".sh().isSuccess

    protected fun uninstall() =
        setupShell() && "run_uninstaller ${AssetHack.apk}".sh().isSuccess

    @WorkerThread
    protected abstract suspend fun operations(): Boolean

    open suspend fun exec() = withContext(Dispatchers.IO) {
        val result = operations()
        if (closeShell)
            shell.close()
        result
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
