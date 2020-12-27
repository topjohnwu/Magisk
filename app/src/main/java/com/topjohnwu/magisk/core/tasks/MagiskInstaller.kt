package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.widget.Toast
import androidx.annotation.WorkerThread
import androidx.core.os.postDelayed
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.events.dialog.EnvFixDialog
import com.topjohnwu.magisk.ktx.reboot
import com.topjohnwu.magisk.ktx.withStreams
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
import org.koin.core.get
import org.koin.core.inject
import timber.log.Timber
import java.io.*
import java.nio.ByteBuffer
import java.security.SecureRandom
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

abstract class MagiskInstallImpl : KoinComponent {

    protected lateinit var installDir: File
    private lateinit var srcBoot: String
    private lateinit var zipUri: Uri

    protected val console: MutableList<String>
    private val logs: MutableList<String>
    private var tarOut: TarOutputStream? = null

    private val service: NetworkService by inject()
    protected val context: Context by inject()

    protected constructor() {
        console = NOPList.getInstance()
        logs = NOPList.getInstance()
    }

    protected constructor(zip: Uri, out: MutableList<String>, err: MutableList<String>) {
        console = out
        logs = err
        zipUri = zip
        installDir = File(get<Context>(Protected).filesDir.parent, "install")
        "rm -rf $installDir".sh()
        installDir.mkdirs()
    }

    private fun findImage(): Boolean {
        srcBoot = "find_boot_image; echo \"\$BOOTIMAGE\"".fsh()
        if (srcBoot.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        console.add("- Target image: $srcBoot")
        return true
    }

    private fun findSecondaryImage(): Boolean {
        val slot = "echo \$SLOT".fsh()
        val target = if (slot == "_a") "_b" else "_a"
        console.add("- Target slot: $target")
        srcBoot = arrayOf(
            "SLOT=$target",
            "find_boot_image",
            "SLOT=$slot",
            "echo \"\$BOOTIMAGE\"").fsh()
        if (srcBoot.isEmpty()) {
            console.add("! Unable to detect target image")
            return false
        }
        console.add("- Target image: $srcBoot")
        return true
    }

    @Suppress("DEPRECATION")
    private fun extractZip(): Boolean {
        val arch = if (Build.VERSION.SDK_INT >= 21) {
            val abis = listOf(*Build.SUPPORTED_ABIS)
            if (abis.contains("x86")) "x86" else "arm"
        } else {
            if (Build.CPU_ABI == "x86") "x86" else "arm"
        }

        console.add("- Device platform: " + Build.CPU_ABI)
        console.add("- Magisk Manager: ${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})")
        console.add("- Install target: ${Info.remote.magisk.version} (${Info.remote.magisk.versionCode})")

        try {
            ZipInputStream(zipUri.inputStream().buffered()).use { zi ->
                lateinit var ze: ZipEntry
                while (zi.nextEntry?.let { ze = it } != null) {
                    if (ze.isDirectory)
                        continue
                    var name: String? = null
                    val names = arrayOf("$arch/", "common/", "META-INF/com/google/android/update-binary")
                    for (n in names) {
                        ze.name.run {
                            if (startsWith(n)) {
                                name = substring(lastIndexOf('/') + 1)
                            }
                        }
                        name ?: continue
                        break
                    }
                    if (name == null && ze.name.startsWith("chromeos/"))
                        name = ze.name
                    name?.also {
                        val dest = if (installDir is SuFile)
                            SuFile(installDir, it)
                        else
                            File(installDir, it)
                        dest.parentFile!!.mkdirs()
                        SuFileOutputStream(dest).use { s -> zi.copyTo(s) }
                    } ?: continue
                }
            }
        } catch (e: IOException) {
            console.add("! Cannot unzip zip")
            Timber.e(e)
            return false
        }

        val init64 = SuFile.open(installDir, "magiskinit64")
        if (Build.VERSION.SDK_INT >= 21 && Build.SUPPORTED_64_BIT_ABIS.isNotEmpty()) {
            init64.renameTo(SuFile.open(installDir, "magiskinit"))
        } else {
            init64.delete()
        }
        "cd $installDir; chmod 755 *".sh()
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
                if (entry.name.contains(".lz4")) LZ4FrameInputStream(tarIn) else tarIn

            while (tarIn.nextEntry?.let { entry = it } != null) {
                if (entry.name.contains("boot.img") ||
                    (Config.recovery && entry.name.contains("recovery.img"))) {
                    val name = entry.name.replace(".lz4", "")
                    console.add("-- Extracting: $name")

                    val extract = File(installDir, name)
                    FileOutputStream(extract).use { decompressedStream().copyTo(it) }
                } else if (entry.name.contains("vbmeta.img")) {
                    val rawData = ByteArrayOutputStream().let {
                        decompressedStream().copyTo(it)
                        it.toByteArray()
                    }
                    // Valid vbmeta.img should be at least 256 bytes
                    if (rawData.size < 256)
                        continue

                    // Patch flags to AVB_VBMETA_IMAGE_FLAGS_VERIFICATION_DISABLED
                    console.add("-- Patching: vbmeta.img")
                    ByteBuffer.wrap(rawData).putInt(120, 2)
                    tarOut.putNextEntry(newEntry("vbmeta.img", rawData.size.toLong()))
                    tarOut.write(rawData)
                } else {
                    console.add("-- Copying: ${entry.name}")
                    tarOut.putNextEntry(entry)
                    tarIn.copyTo(tarOut, bufferSize = 1024 * 1024)
                }
            }
            val boot = SuFile.open(installDir, "boot.img")
            val recovery = SuFile.open(installDir, "recovery.img")
            if (recovery.exists() && boot.exists()) {
                // Install Magisk to recovery
                srcBoot = recovery.path
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
                srcBoot = boot.path
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
                val alphaNum = "$alpha${alpha.toUpperCase(Locale.ROOT)}0123456789"
                val random = SecureRandom()
                val suffix = StringBuilder()
                for (i in 1..5) {
                    suffix.append(alphaNum[random.nextInt(alphaNum.length)])
                }

                val filename = "magisk_patched_$suffix"
                outStream = if (magic.contentEquals("ustar".toByteArray())) {
                    outFile = MediaStoreUtils.getFile("$filename.tar", true)
                    handleTar(src, outFile!!.uri.outputStream())
                } else {
                    // Raw image
                    srcBoot = File(installDir, "boot.img").path
                    console.add("- Copying image to cache")
                    FileOutputStream(srcBoot).use { src.copyTo(it) }
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
            val patched = SuFile.open(installDir, "new-boot.img")
            if (outStream is TarOutputStream) {
                val name = if (srcBoot.contains("recovery")) "recovery.img" else "boot.img"
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

        return true
    }

    private fun patchBoot(): Boolean {
        var srcNand = ""
        if ("[ -c $srcBoot ] && nanddump -f boot.img $srcBoot".sh().isSuccess) {
            srcNand = srcBoot
            srcBoot = File(installDir, "boot.img").path
        }

        var isSigned: Boolean
        try {
            SuFileInputStream(srcBoot).use {
                isSigned = SignBoot.verifySignature(it, null)
                if (isSigned) {
                    console.add("- Boot image is signed with AVB 1.0")
                }
            }
        } catch (e: IOException) {
            console.add("! Unable to check signature")
            return false
        }

        if (!("KEEPFORCEENCRYPT=${Config.keepEnc} KEEPVERITY=${Config.keepVerity} " +
                "RECOVERYMODE=${Config.recovery} sh update-binary " +
                "sh boot_patch.sh $srcBoot").sh().isSuccess) {
            return false
        }

        if (srcNand.isNotEmpty()) {
            srcBoot = srcNand
        }

        val job = Shell.sh(
            "./magiskboot cleanup",
            "mv bin/busybox busybox",
            "rm -rf magisk.apk bin boot.img update-binary",
            "cd /")

        val patched = File(installDir, "new-boot.img")
        if (isSigned) {
            console.add("- Signing boot image with verity keys")
            val signed = File(installDir, "signed.img")
            try {
                withStreams(SuFileInputStream(patched), signed.outputStream().buffered()) {
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

    private fun copySepolicyRules(): Boolean {
        if (Info.remote.magisk.versionCode >= 21100) return true
        // Copy existing rules for migration
        "copy_sepolicy_rules".sh()
        return true
    }

    private fun flashBoot(): Boolean {
        if (!"direct_install $installDir $srcBoot".sh().isSuccess)
            return false
        "run_migrations".sh()
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

    private fun String.sh() = Shell.sh(this).to(console, logs).exec()
    private fun Array<String>.sh() = Shell.sh(*this).to(console, logs).exec()
    private fun String.fsh() = ShellUtils.fastCmd(this)
    private fun Array<String>.fsh() = ShellUtils.fastCmd(*this)

    protected fun doPatchFile(patchFile: Uri) = extractZip() && handleFile(patchFile)

    protected fun direct() = findImage() && extractZip() && patchBoot() &&
        copySepolicyRules() && flashBoot()

    protected suspend fun secondSlot() = findSecondaryImage() && extractZip() &&
        patchBoot() && copySepolicyRules() && flashBoot() && postOTA()

    protected fun fixEnv(zip: Uri): Boolean {
        installDir = SuFile("/data/adb/magisk")
        Shell.su("rm -rf /data/adb/magisk/*").exec()
        zipUri = zip
        return extractZip() && Shell.su("fix_env").exec().isSuccess
    }

    @WorkerThread
    protected abstract suspend fun operations(): Boolean

    open suspend fun exec() = withContext(Dispatchers.IO) { operations() }
}

sealed class MagiskInstaller(
    file: Uri,
    console: MutableList<String>,
    logs: MutableList<String>
) : MagiskInstallImpl(file, console, logs) {

    override suspend fun exec(): Boolean {
        val success = super.exec()
        if (success) {
            console.add("- All done!")
        } else {
            Shell.sh("rm -rf $installDir").submit()
            console.add("! Installation failed")
        }
        return success
    }

    class Patch(
        file: Uri,
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(file, console, logs) {
        override suspend fun operations() = doPatchFile(uri)
    }

    class SecondSlot(
        file: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(file, console, logs) {
        override suspend fun operations() = secondSlot()
    }

    class Direct(
        file: Uri,
        console: MutableList<String>,
        logs: MutableList<String>
    ) : MagiskInstaller(file, console, logs) {
        override suspend fun operations() = direct()
    }

}

class EnvFixTask(
    private val zip: Uri
) : MagiskInstallImpl() {
    override suspend fun operations() = fixEnv(zip)

    override suspend fun exec(): Boolean {
        val success = super.exec()
        LocalBroadcastManager.getInstance(context).sendBroadcast(Intent(EnvFixDialog.DISMISS))
        Utils.toast(
            if (success) R.string.reboot_delay_toast else R.string.setup_fail,
            Toast.LENGTH_LONG
        )
        if (success)
            UiThreadHandler.handler.postDelayed(5000) { reboot() }
        return success
    }
}
