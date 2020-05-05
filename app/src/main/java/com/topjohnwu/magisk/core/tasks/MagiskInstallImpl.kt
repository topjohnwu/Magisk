package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.net.Uri
import android.os.Build
import android.text.TextUtils
import androidx.annotation.WorkerThread
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.signing.SignBoot
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils
import com.topjohnwu.superuser.internal.NOPList
import com.topjohnwu.superuser.io.SuFile
import com.topjohnwu.superuser.io.SuFileInputStream
import com.topjohnwu.superuser.io.SuFileOutputStream
import io.reactivex.Single
import org.kamranzafar.jtar.TarEntry
import org.kamranzafar.jtar.TarHeader
import org.kamranzafar.jtar.TarInputStream
import org.kamranzafar.jtar.TarOutputStream
import timber.log.Timber
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.nio.ByteBuffer
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

abstract class MagiskInstallImpl : FlashResultListener {

    protected lateinit var installDir: File
    private lateinit var srcBoot: String
    private lateinit var destFile: File
    private lateinit var zipUri: Uri

    private val console: MutableList<String>
    private val logs: MutableList<String>
    private var tarOut: TarOutputStream? = null

    private val service: GithubRawServices by inject()
    protected val context: Context by inject()

    protected constructor() {
        console = NOPList.getInstance()
        logs = NOPList.getInstance()
    }

    constructor(zip: Uri, out: MutableList<String>, err: MutableList<String>) {
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

    private fun extractZip(): Boolean {
        val arch: String
        arch = if (Build.VERSION.SDK_INT >= 21) {
            val abis = listOf(*Build.SUPPORTED_ABIS)
            if (abis.contains("x86")) "x86" else "arm"
        } else {
            if (TextUtils.equals(Build.CPU_ABI, "x86")) "x86" else "arm"
        }

        console.add("- Device platform: " + Build.CPU_ABI)

        try {
            ZipInputStream(context.readUri(zipUri).buffered()).use { zi ->
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
                    if (name == null)
                        continue
                    val dest = if (installDir is SuFile)
                        SuFile(installDir, name)
                    else
                        File(installDir, name)
                    dest.parentFile!!.mkdirs()
                    SuFileOutputStream(dest).use { zi.copyTo(it) }
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
    private fun handleTar(input: InputStream) {
        console.add("- Processing tar file")
        var vbmeta = false
        val tarOut = TarOutputStream(destFile)
        this.tarOut = tarOut
        TarInputStream(input).use { tarIn ->
            lateinit var entry: TarEntry
            while (tarIn.nextEntry?.let { entry = it } != null) {
                if (entry.name.contains("boot.img") || entry.name.contains("recovery.img")) {
                    val name = entry.name
                    console.add("-- Extracting: $name")
                    val extract = File(installDir, name)
                    FileOutputStream(extract).use { tarIn.copyTo(it) }
                    if (name.contains(".lz4")) {
                        console.add("-- Decompressing: $name")
                        "./magiskboot decompress $extract".sh()
                    }
                } else if (entry.name.contains("vbmeta.img")) {
                    vbmeta = true
                    val buf = ByteBuffer.allocate(256)
                    buf.put("AVB0".toByteArray())   // magic
                    buf.putInt(1)                   // required_libavb_version_major
                    buf.putInt(120, 2)              // flags
                    buf.position(128)               // release_string
                    buf.put("avbtool 1.1.0".toByteArray())
                    tarOut.putNextEntry(newEntry("vbmeta.img", 256))
                    tarOut.write(buf.array())
                } else {
                    console.add("-- Writing: " + entry.name)
                    tarOut.putNextEntry(entry)
                    tarIn.copyTo(tarOut)
                }
            }
            val boot = SuFile.open(installDir, "boot.img")
            val recovery = SuFile.open(installDir, "recovery.img")
            if (vbmeta && recovery.exists() && boot.exists()) {
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
    }

    private fun handleFile(uri: Uri): Boolean {
        try {
            context.readUri(uri).buffered().use {
                it.mark(500)
                val magic = ByteArray(5)
                if (it.skip(257) != 257L || it.read(magic) != magic.size) {
                    console.add("! Invalid file")
                    return false
                }
                it.reset()
                if (magic.contentEquals("ustar".toByteArray())) {
                    destFile = File(Config.downloadDirectory, "magisk_patched.tar")
                    handleTar(it)
                } else {
                    // Raw image
                    srcBoot = File(installDir, "boot.img").path
                    destFile = File(Config.downloadDirectory, "magisk_patched.img")
                    console.add("- Copying image to cache")
                    FileOutputStream(srcBoot).use { out -> it.copyTo(out) }
                }
            }
        } catch (e: IOException) {
            console.add("! Process error")
            Timber.e(e)
            return false
        }

        return true
    }

    private fun patchBoot(): Boolean {
        var isSigned = false
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

        if (!("KEEPFORCEENCRYPT=${Info.keepEnc} KEEPVERITY=${Info.keepVerity} " +
                "RECOVERYMODE=${Info.recovery} sh update-binary " +
                "sh boot_patch.sh $srcBoot").sh().isSuccess) {
            return false
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
                    input, out -> SignBoot.doSignature("/boot", input, out, null, null)
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
        arrayOf("run_migrations").sh()
        return true
    }

    private fun storeBoot(): Boolean {
        val patched = SuFile.open(installDir, "new-boot.img")
        try {
            val os = tarOut?.let {
                it.putNextEntry(newEntry(
                    if (srcBoot.contains("recovery")) "recovery.img" else "boot.img",
                    patched.length()))
                tarOut = null
                it
            } ?: destFile.outputStream()
            patched.suInputStream().use { it.copyTo(os); os.close() }
        } catch (e: IOException) {
            console.add("! Failed to output to $destFile")
            Timber.e(e)
            return false
        }

        patched.delete()
        console.add("")
        console.add("****************************")
        console.add(" Output file is placed in ")
        console.add(" $destFile ")
        console.add("****************************")
        return true
    }

    private fun postOTA(): Boolean {
        val bootctl = SuFile("/data/adb/bootctl")
        try {
            withStreams(service.fetchBootctl().blockingGet().byteStream(), bootctl.suOutputStream()) {
                input, out -> input.copyTo(out)
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

    protected fun doPatchFile(patchFile: Uri) =
        extractZip() && handleFile(patchFile) && patchBoot() && storeBoot()

    protected fun direct() = findImage() && extractZip() && patchBoot() && flashBoot()

    protected fun secondSlot() =
        findSecondaryImage() && extractZip() && patchBoot() && flashBoot() && postOTA()

    protected fun fixEnv(zip: File): Boolean {
        installDir = SuFile("/data/adb/magisk")
        Shell.su("rm -rf /data/adb/magisk/*").exec()
        zipUri = zip.toUri()
        return extractZip() && Shell.su("fix_env").exec().isSuccess
    }

    @WorkerThread
    protected abstract fun operations(): Boolean

    fun exec() {
        Single.fromCallable { operations() }.subscribeK { onResult(it) }
    }

}
