package com.topjohnwu.magisk.test

import androidx.annotation.Keep
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.repository.FirmwareCrawler
import com.topjohnwu.magisk.core.tasks.OtaPayloadExtractor
import com.topjohnwu.magisk.core.tasks.PatchFileClassifier
import com.topjohnwu.magisk.core.tasks.RecoveryGptProcessor
import com.topjohnwu.magisk.core.tasks.TarProcessor
import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.magisk.core.utils.ZipChannel
import com.topjohnwu.superuser.nio.ExtendedFile
import com.topjohnwu.superuser.nio.FileSystemManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer
import java.security.DigestInputStream
import java.security.DigestOutputStream
import java.security.MessageDigest
import java.util.zip.CRC32
import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream
import kotlin.random.Random

private const val FACTORY_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-cd1a.261005.003.b1-factory-90f36052.zip"
private const val OTA_IMAGE_URL =
    "https://dl.google.com/dl/android/aosp/yogi-ota-cd1a.261005.003.b1-7d7d9988.zip"
private const val TAR_IMAGE_URL =
    "https://github.com/topjohnwu/magisk-files/releases/download/files/AP_F968B_trimmed.tar"
private const val AL_RECOVERY_URL =
    "https://dl.google.com/device/recovery/mica-user/16471258/recovery.zip"
private const val INIT_BOOT_SIZE = 8388608L
private const val INIT_BOOT_SHA256 =
    "01ef3679b997989281309e1c0ad96c433da206b822b8db8408f8b474f71405b8"
private const val TAR_INIT_BOOT_SHA256 =
    "bd4b3629fd483701395540ed207e3d64cf51757d9a63a967d24ba8da86ae026c"
private const val AL_INIT_BOOT_SHA256 =
    "20fe3b6572a7b400ad6491c759d3baa3231cdd9961d43476ae64e7f43380329f"
private const val AL_VENDOR_BOOT_SHA256 =
    "a400d6b0a542eb226443d8f82a6c4b26eafe9e212955c668631b4bb9962c8059"

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

    // Classify the file at url, and extract the boot image with the extractor
    private fun testZipExtraction(
        url: String,
        expectedType: PatchFileClassifier.Type,
        extract: suspend (PatchFileClassifier, MutableList<String>, MutableList<String>) -> Unit,
    ) = runBlocking(Dispatchers.IO) {
        val console = mutableListOf<String>()
        val logs = mutableListOf<String>()

        try {
            DataChannel.Http(ServiceLocator.okhttp, url).use { channel ->
                PatchFileClassifier(channel).use { file ->
                    assertEquals("Unexpected file type", expectedType, file.type)
                    extract(file, console, logs)
                }
            }
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
    fun testFactoryZipExtraction() =
        testZipExtraction(FACTORY_IMAGE_URL, PatchFileClassifier.Type.BootZip) { file, _, _ ->
            file.openStream().use { input -> outFile.outputStream().use { input.copyTo(it) } }
        }

    @Test
    fun testOtaZipExtraction() =
        testZipExtraction(OTA_IMAGE_URL, PatchFileClassifier.Type.PayloadBin) { file, console, logs ->
            OtaPayloadExtractor(outFile, console, logs).extract(file.openChannel())
        }

    @Test
    fun testTarProcessor() = runBlocking(Dispatchers.IO) {
        val workingDir = FileSystemManager.getLocal().getFile(tarWorkingDir.path)
        val console = mutableListOf<String>()
        val logs = mutableListOf<String>()

        try {
            FileOutputStream("/dev/null").buffered(1024 * 1024).use { outStream ->
                val tar = TarProcessor(workingDir, outStream, console, logs)
                val bootImage = DataChannel.Http(ServiceLocator.okhttp, TAR_IMAGE_URL).use { channel ->
                    PatchFileClassifier(channel).use { file ->
                        assertEquals("Unexpected file type", PatchFileClassifier.Type.Tar, file.type)
                        tar.start(file.openStream())
                    }
                }

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

    @Test
    fun testRecoveryGptProcessor() = runBlocking(Dispatchers.IO) {
        val dir = File(appContext.cacheDir, "test_gpt_dir")
        dir.deleteRecursively()
        dir.mkdirs()
        val workingDir = FileSystemManager.getLocal().getFile(dir.path)
        val console = mutableListOf<String>()
        val logs = mutableListOf<String>()

        var initBootVerified = false
        var vendorBootVerified = false

        // Verify the images without patching, so the output should be identical to the input
        class VerifyProcessor(out: OutputStream) :
            RecoveryGptProcessor(workingDir, out, console, logs) {

            override suspend fun onInitBoot(image: ExtendedFile) {
                assertEquals("Checksum mismatch for init_boot_a", AL_INIT_BOOT_SHA256, calculateSha256(image))
                initBootVerified = true
            }

            override suspend fun onVendorBoot(image: ExtendedFile) {
                assertEquals("Checksum mismatch for vendor_boot_b", AL_VENDOR_BOOT_SHA256, calculateSha256(image))
                vendorBootVerified = true
            }
        }

        try {
            val inputDigest = MessageDigest.getInstance("SHA-256")
            val outputDigest = MessageDigest.getInstance("SHA-256")

            // The recovery image is too large to store on device. Stream and decompress
            // the image on-the-fly, and only calculate checksums of the input and output.
            DataChannel.Http(ServiceLocator.okhttp, AL_RECOVERY_URL).use { http ->
                PatchFileClassifier(http).use { file ->
                    assertEquals("Unexpected file type", PatchFileClassifier.Type.RecoveryGpt, file.type)
                    val output = DigestOutputStream(FileOutputStream("/dev/null"), outputDigest)
                    output.buffered(1024 * 1024).use { outStream ->
                        val input = DigestInputStream(file.openStream(), inputDigest)
                        val processor = VerifyProcessor(outStream)
                        val bootImage = processor.start(input)

                        assertTrue("Extracted init_boot_a should exist", bootImage.exists())
                        assertEquals(
                            "Extracted file size should be $INIT_BOOT_SIZE bytes",
                            INIT_BOOT_SIZE,
                            bootImage.length()
                        )

                        processor.finish(bootImage)
                    }
                }
            }

            assertTrue("init_boot_a was not processed", initBootVerified)
            assertTrue("vendor_boot_b was not processed", vendorBootVerified)
            assertArrayEquals(
                "Output should be identical to the input",
                inputDigest.digest(),
                outputDigest.digest()
            )
        } catch (e: Throwable) {
            System.err.println("Console:\n" + console.joinToString("\n"))
            System.err.println("Logs:\n" + logs.joinToString("\n"))
            throw e
        } finally {
            dir.deleteRecursively()
        }
    }

    @Test
    fun testZipChannelEntries() {
        val zip = buildZip(
            ZipItem("dir/", ByteArray(0), ZipEntry.STORED),
            ZipItem("dir/boot.img", testData(1000, 1), ZipEntry.STORED),
            ZipItem("dir/sub/", ByteArray(0), ZipEntry.STORED),
            ZipItem("payload.bin", testData(2000, 2), ZipEntry.DEFLATED),
        )
        val source = ByteArrayChannel(zip)
        ZipChannel(source).use { zc ->
            assertEquals(
                "Directories should be excluded",
                listOf("dir/boot.img", "payload.bin"),
                zc.entries.map { it.name }
            )
            assertNotNull("getEntry should find files", zc.getEntry("payload.bin"))
            assertNull("getEntry should return null for missing files", zc.getEntry("missing"))
            assertEquals("dir/boot.img", zc.find { it.name.endsWith("boot.img") }?.name)
            assertNull(zc.find { it.name.endsWith(".zip") })
        }
        assertTrue("ZipChannel should not close the source channel", source.isOpen)
    }

    @Test
    fun testZipChannelStored() {
        // Larger than all DataChannel cache thresholds
        val data = testData(3 * 1024 * 1024 + 123, 3)
        val zip = buildZip(
            ZipItem("first.txt", "hello".toByteArray(), ZipEntry.DEFLATED),
            ZipItem("image.img", data, ZipEntry.STORED),
        )
        ZipChannel(ByteArrayChannel(zip)).use { zc ->
            val entry = zc.getEntry("image.img")!!

            // Random access through open()
            zc.open(entry).use { ch ->
                assertEquals("Channel size mismatch", data.size.toLong(), ch.size())
                for (offset in listOf(0, 1, 4095, 16 * 1024, 700 * 1024, data.size - 100)) {
                    for (len in listOf(1, 100, 64 * 1024, 600 * 1024)) {
                        val n = minOf(len, data.size - offset)
                        val buf = ByteBuffer.allocate(n)
                        assertEquals(n, ch.read(buf, offset.toLong()))
                        assertArrayEquals(
                            "Random read mismatch at $offset+$n",
                            data.copyOfRange(offset, offset + n),
                            buf.array()
                        )
                    }
                }
                assertEquals("Read beyond EOF", -1, ch.read(ByteBuffer.allocate(1), data.size.toLong()))
                ch.stream().use { assertArrayEquals("Channel stream mismatch", data, it.readBytes()) }
                ch.slice(1000, 5000).stream().use {
                    assertArrayEquals("Slice mismatch", data.copyOfRange(1000, 6000), it.readBytes())
                }
            }

            // Sequential access through openStream()
            zc.openStream(entry).use { assertArrayEquals("Stream mismatch", data, it.readBytes()) }
        }
    }

    @Test
    fun testZipChannelDeflated() {
        val random = testData(2 * 1024 * 1024, 4)
        val text = "Magisk ".repeat(100000).toByteArray()
        val zip = buildZip(
            ZipItem("random.bin", random, ZipEntry.DEFLATED),
            ZipItem("text.txt", text, ZipEntry.DEFLATED),
        )
        ZipChannel(ByteArrayChannel(zip)).use { zc ->
            for ((name, data) in listOf("random.bin" to random, "text.txt" to text)) {
                val entry = zc.getEntry(name)!!
                assertEquals("Uncompressed size mismatch", data.size.toLong(), entry.size)
                zc.openStream(entry).use {
                    assertArrayEquals("Decompressed $name mismatch", data, it.readBytes())
                }
                assertThrows("open() should reject compressed entries", IOException::class.java) {
                    zc.open(entry)
                }
            }
            // Text compresses well, make sure it is really compressed
            assertTrue(zc.getEntry("text.txt")!!.compressedSize < text.size / 10)
        }
    }

    @Test
    fun testZipChannelEmptyEntry() {
        val zip = buildZip(
            ZipItem("empty.stored", ByteArray(0), ZipEntry.STORED),
            ZipItem("empty.deflated", ByteArray(0), ZipEntry.DEFLATED),
        )
        ZipChannel(ByteArrayChannel(zip)).use { zc ->
            val stored = zc.getEntry("empty.stored")!!
            assertThrows(IOException::class.java) { zc.open(stored) }
            zc.openStream(stored).use { assertEquals(0, it.readBytes().size) }
            zc.openStream(zc.getEntry("empty.deflated")!!).use { assertEquals(0, it.readBytes().size) }
        }
    }

    @Test
    fun testZipChannelNested() {
        val boot = testData(512 * 1024, 5)
        val payload = testData(256 * 1024, 6)
        val inner = buildZip(
            ZipItem("init_boot.img", boot, ZipEntry.DEFLATED),
            ZipItem("payload.bin", payload, ZipEntry.STORED),
        )
        val outer = buildZip(
            ZipItem("README", "readme".toByteArray(), ZipEntry.DEFLATED),
            ZipItem("device/image-device-1234.zip", inner, ZipEntry.STORED),
        )
        ZipChannel(ByteArrayChannel(outer)).use { zc ->
            val innerEntry = zc.find { it.name.endsWith(".zip") }!!
            zc.open(innerEntry).use { innerChannel ->
                ZipChannel(innerChannel).use { izc ->
                    assertEquals(listOf("init_boot.img", "payload.bin"), izc.entries.map { it.name })
                    izc.openStream(izc.getEntry("init_boot.img")!!).use {
                        assertArrayEquals("Nested deflated entry mismatch", boot, it.readBytes())
                    }
                    izc.open(izc.getEntry("payload.bin")!!).use { ch ->
                        val buf = ByteBuffer.allocate(1000)
                        ch.read(buf, 5000)
                        assertArrayEquals(
                            "Nested stored entry mismatch",
                            payload.copyOfRange(5000, 6000),
                            buf.array()
                        )
                    }
                }
                assertTrue("Inner ZipChannel should not close its source", innerChannel.isOpen)
            }
        }
    }

    @Test
    fun testZipChannelInvalid() {
        assertThrows(IOException::class.java) {
            ZipChannel(ByteArrayChannel(testData(4096, 7)))
        }
    }

    @Test
    fun testPixelProvider() = runBlocking(Dispatchers.IO) {
        val svc = ServiceLocator.networkService

        // Match latest
        val candidate = FirmwareCrawler.PixelProvider.crawl(svc, "shiba", "Pixel 8")
        assertNotNull(candidate)
        assertTrue(candidate!!.url.startsWith("https://dl.google.com/dl/android/aosp/"))
        assertTrue(candidate.url.endsWith(".zip"))
        assertTrue(candidate.description.startsWith("Pixel 8 •"))

        // Match specific build ID
        val candidateSpecific =
            FirmwareCrawler.PixelProvider.crawl(svc, "shiba", "Pixel 8", "UD1A.230803.022.A5")
        assertNotNull(candidateSpecific)
        assertTrue(candidateSpecific!!.url.contains("shiba-ud1a.230803.022.a5"))
        assertTrue(candidateSpecific.description.contains("UD1A.230803.022.A5"))

        // Non-pixel device should return null
        val candidateNexus = FirmwareCrawler.PixelProvider.crawl(svc, "angler", "Nexus 6P")
        assertNull(candidateNexus)

        // Unknown device should return null
        val candidateUnknown = FirmwareCrawler.PixelProvider.crawl(svc, "unknown", "Unknown")
        assertNull(candidateUnknown)
    }

    private class ZipItem(val name: String, val data: ByteArray, val method: Int)

    private fun buildZip(vararg items: ZipItem): ByteArray {
        val bytes = ByteArrayOutputStream()
        ZipOutputStream(bytes).use { zip ->
            for (item in items) {
                val entry = ZipEntry(item.name)
                entry.method = item.method
                if (item.method == ZipEntry.STORED) {
                    // STORED entries require size and CRC to be set beforehand
                    entry.size = item.data.size.toLong()
                    entry.compressedSize = item.data.size.toLong()
                    entry.crc = CRC32().apply { update(item.data) }.value
                }
                zip.putNextEntry(entry)
                zip.write(item.data)
                zip.closeEntry()
            }
        }
        return bytes.toByteArray()
    }

    private fun testData(size: Int, seed: Long) = ByteArray(size).also { Random(seed).nextBytes(it) }

    // An in-memory DataChannel
    private class ByteArrayChannel(
        private val data: ByteArray,
        startOffset: Long = 0,
        size: Long = data.size.toLong(),
    ) : DataChannel(startOffset, size) {
        override fun slice(offset: Long, sliceSize: Long): DataChannel {
            require(offset >= 0 && sliceSize >= 0 && offset + sliceSize <= size)
            return ByteArrayChannel(data, startOffset + offset, sliceSize)
        }

        override fun stream(offset: Long, sliceSize: Long): InputStream {
            val len = minOf(offset + sliceSize, size) - offset
            return ByteArrayInputStream(data, (startOffset + offset).toInt(), len.toInt())
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
