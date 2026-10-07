package com.topjohnwu.magisk.test

import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.tasks.TarProcessor
import com.topjohnwu.magisk.core.tasks.ZipExtractor
import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream
import java.security.MessageDigest

private const val FACTORY_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-cd1a.261005.003.b1-factory-90f36052.zip"
private const val OTA_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-ota-cd1a.261005.003.b1-7d7d9988.zip"
private const val TAR_IMAGE_URL =
    "https://github.com/topjohnwu/magisk-files/releases/download/files/AP_F968B_trimmed.tar"
private const val INIT_BOOT_SIZE = 8388608L
private const val INIT_BOOT_SHA256 =
    "01ef3679b997989281309e1c0ad96c433da206b822b8db8408f8b474f71405b8"
private const val TAR_INIT_BOOT_SHA256 =
    "bd4b3629fd483701395540ed207e3d64cf51757d9a63a967d24ba8da86ae026c"

@Keep
@RunWith(AndroidJUnit4::class)
class AppTest : TestCommon {

    private lateinit var outFile: File
    private lateinit var tarWorkingDir: File

    @Before
    fun setup() {
        outFile = File(appContext.cacheDir, "test_init_boot.img")
        outFile.delete()
        tarWorkingDir = File(appContext.cacheDir, "test_tar_dir")
        tarWorkingDir.deleteRecursively()
        tarWorkingDir.mkdirs()
    }

    @After
    fun teardown() {
        outFile.delete()
        tarWorkingDir.deleteRecursively()
    }

    private fun testZipExtraction(url: String) = runBlocking(Dispatchers.IO) {
        val console = mutableListOf<String>()
        val logs = mutableListOf<String>()

        try {
            val channel = DataChannel.Http(ServiceLocator.okhttp, url)
            ZipExtractor(outFile, console, logs).extract(channel)
        } catch (e: Exception) {
            System.err.println("Console:\n" + console.joinToString("\n"))
            System.err.println("Logs:\n" + logs.joinToString("\n"))
            throw e
        }

        assertTrue("Extracted init_boot.img should exist", outFile.exists())
        assertEquals("Extracted file size should be $INIT_BOOT_SIZE bytes", INIT_BOOT_SIZE, outFile.length())

        val actualSha256 = calculateSha256(outFile)
        assertEquals("Checksum mismatch for extracted init_boot.img", INIT_BOOT_SHA256, actualSha256)
    }

    @Test
    fun testFactoryZipExtraction() = testZipExtraction(FACTORY_IMAGE_URL)

    @Test
    fun testOtaZipExtraction() = testZipExtraction(OTA_IMAGE_URL)

    @Test
    fun testTarProcessor() = runBlocking(Dispatchers.IO) {
        val workingDir = FileSystemManager.getLocal().getFile(tarWorkingDir.path)
        val console = mutableListOf<String>()
        val logs = mutableListOf<String>()

        try {
            FileOutputStream("/dev/null").buffered(1024 * 1024).use { outStream ->
                val channel = DataChannel.Http(ServiceLocator.okhttp, TAR_IMAGE_URL)
                val tar = TarProcessor(workingDir, outStream, console, logs)
                val bootImage = tar.start(channel)

                assertTrue("Extracted init_boot.img should exist", bootImage.exists())
                assertEquals("Extracted file size should be $INIT_BOOT_SIZE bytes", INIT_BOOT_SIZE, bootImage.length())

                val actualSha256 = calculateSha256(bootImage)
                assertEquals("Checksum mismatch for extracted init_boot.img", TAR_INIT_BOOT_SHA256, actualSha256)

                tar.finish(bootImage)
            }
        } catch (e: Exception) {
            System.err.println("Console:\n" + console.joinToString("\n"))
            System.err.println("Logs:\n" + logs.joinToString("\n"))
            throw e
        }
    }

    private fun calculateSha256(file: File): String =
        file.inputStream().use { calculateSha256(it) }

    private fun calculateSha256(file: ExtendedFile): String =
        file.newInputStream().use { calculateSha256(it) }

    private fun calculateSha256(input: InputStream): String {
        val digest = MessageDigest.getInstance("SHA-256")
        val buffer = ByteArray(64 * 1024)
        var bytesRead: Int
        while (input.read(buffer).also { bytesRead = it } != -1) {
            digest.update(buffer, 0, bytesRead)
        }
        return digest.digest().joinToString("") { "%02x".format(it) }
    }
}
