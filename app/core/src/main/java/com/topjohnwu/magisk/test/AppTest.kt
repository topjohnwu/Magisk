package com.topjohnwu.magisk.test

import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.tasks.ZipExtractor
import com.topjohnwu.magisk.core.utils.DataChannel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.security.MessageDigest

private const val FACTORY_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-cd1a.261005.003.b1-factory-90f36052.zip"
private const val OTA_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-ota-cd1a.261005.003.b1-7d7d9988.zip"
private const val INIT_BOOT_SIZE = 8388608L
private const val INIT_BOOT_SHA256 =
    "01ef3679b997989281309e1c0ad96c433da206b822b8db8408f8b474f71405b8"

@Keep
@RunWith(AndroidJUnit4::class)
class AppTest : TestCommon {

    private lateinit var outFile: File

    @Before
    fun setup() {
        outFile = File(appContext.cacheDir, "test_init_boot.img")
        outFile.delete()
    }

    @After
    fun teardown() {
        outFile.delete()
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

    private fun calculateSha256(file: File): String {
        val digest = MessageDigest.getInstance("SHA-256")
        file.inputStream().use { input ->
            val buffer = ByteArray(64 * 1024)
            var bytesRead: Int
            while (input.read(buffer).also { bytesRead = it } != -1) {
                digest.update(buffer, 0, bytesRead)
            }
        }
        return digest.digest().joinToString("") { "%02x".format(it) }
    }
}
