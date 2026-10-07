package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.nio.ExtendedFile
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.withContext
import java.io.EOFException
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.zip.CRC32

/**
 * Patch Aluminium OS recovery images (GPT disk images, e.g. recovery_image.bin).
 *
 * A single-use, two-phase processor; the caller patches the boot image in between:
 * 1. [start]: copy the input to the output until init_boot_a is reached, and extract
 *    init_boot_a into [workingDir]. Returns the image to be patched.
 * 2. [finish]: write the patched image to the output, and copy the rest of the input.
 *
 * In addition, vendor_boot_b is always patched to disable data wipe and AVB hash
 * verification (see `patch_al_vendor_boot` in app_functions.sh) when encountered.
 *
 * Subclasses can override [onInitBoot] and [onVendorBoot] to customize
 * how the images are processed before being written to the output.
 *
 * The input is processed in a single sequential pass, so it can be streamed from
 * any source. The partition table is left untouched, and the output has exactly
 * the same size as the input.
 *
 * [finish] or [close] closes the input stream. The caller owns the output stream
 * and is responsible for closing it.
 */
open class RecoveryGptProcessor(
    private val workingDir: ExtendedFile,
    private val output: OutputStream,
    private val console: MutableList<String>,
    private val logs: MutableList<String>,
) : WholeFilePatcher {

    class InvalidImageException(msg: String) : IOException(msg)

    private class Partition(val name: String, firstLba: Long, lastLba: Long) {
        val offset = firstLba * SECTOR_SIZE
        val size = (lastLba - firstLba + 1) * SECTOR_SIZE
    }

    private var source: InputStream? = null
    private val input get() = source ?: throw IOException("Input stream closed")
    private lateinit var partitions: Iterator<Partition>
    private lateinit var target: Partition
    private val buffer = ByteArray(BUFFER_SIZE)

    // Number of bytes consumed from the input
    private var position = 0L

    @Throws(IOException::class)
    override suspend fun start(input: InputStream): ExtendedFile {
        source = input
        console.add("- Processing Aluminium OS recovery image")
        partitions = readPartitionTable().iterator()

        // Copy everything until init_boot_a is found
        while (partitions.hasNext()) {
            val part = partitions.next()
            if (part.name == INIT_BOOT) {
                copy(part.offset - position)
                console.add("-- Extracting: $INIT_BOOT")
                target = part
                val file = workingDir.getChildFile("$INIT_BOOT.img")
                file.newOutputStream().use { extract(part.size, it) }
                onInitBoot(file)
                return file
            }
            processPartition(part)
        }
        throw InvalidImageException("$INIT_BOOT not found")
    }

    @Throws(IOException::class)
    override suspend fun finish(patched: ExtendedFile) {
        use {
            writeImage(target, patched)
            while (partitions.hasNext()) {
                processPartition(partitions.next())
            }
            // Copy the remaining data, including the backup GPT
            console.add("-- Copying remaining data")
            copy(Long.MAX_VALUE)
            output.flush()
        }
    }

    override fun close() {
        source?.let {
            source = null
            runCatching { it.close() }
        }
    }

    private suspend fun processPartition(part: Partition) {
        copy(part.offset - position)
        if (part.name == VENDOR_BOOT) {
            console.add("-- Patching  : $VENDOR_BOOT")
            val image = workingDir.getChildFile("$VENDOR_BOOT.img")
            try {
                image.newOutputStream().use { extract(part.size, it) }
                onVendorBoot(image)
                writeImage(part, image)
            } finally {
                image.delete()
            }
        } else {
            logs.add("Copying: ${part.name}")
            copy(part.size)
        }
    }

    /**
     * Called when encounter the init_boot_a [image].
     * No additional processing is done by default.
     */
    @Throws(IOException::class)
    protected open suspend fun onInitBoot(image: ExtendedFile) {}

    /**
     * Called when encounter the vendor_boot_b [image].
     * By default, the image is patched with `patch_al_vendor_boot` in [workingDir].
     */
    @Throws(IOException::class)
    protected open suspend fun onVendorBoot(image: ExtendedFile) {
        val success = Shell.cmd("patch_al_vendor_boot $workingDir $image")
            .to(console, logs).exec().isSuccess
        if (!success) {
            throw IOException("Failed to patch $VENDOR_BOOT")
        }
    }

    // Write the image to the partition, zero padding to the partition size
    private suspend fun writeImage(part: Partition, image: ExtendedFile) {
        val size = image.length()
        if (size > part.size) {
            throw IOException("${part.name}: image size $size exceeds partition size ${part.size}")
        }
        console.add("-- Writing   : ${part.name}")

        withContext(Dispatchers.IO) {
            var written = 0L
            image.newInputStream().use {
                while (written < size) {
                    ensureActive()
                    val n = it.read(buffer, 0, minOf(size - written, BUFFER_SIZE.toLong()).toInt())
                    if (n < 0) throw EOFException("${part.name}: unexpected end of image")
                    output.write(buffer, 0, n)
                    written += n
                }
            }
            buffer.fill(0)
            while (written < part.size) {
                ensureActive()
                val n = minOf(part.size - written, BUFFER_SIZE.toLong()).toInt()
                output.write(buffer, 0, n)
                written += n
            }
        }
    }

    private suspend fun readPartitionTable(): List<Partition> {
        // LBA 0 is the protective MBR, LBA 1 is the GPT header
        val head = ByteArray(2 * SECTOR_SIZE.toInt())
        readFully(head)
        output.write(head)
        position += head.size

        val hdr = ByteBuffer.wrap(head, SECTOR_SIZE.toInt(), SECTOR_SIZE.toInt())
            .slice().order(ByteOrder.LITTLE_ENDIAN)
        if (!isGpt(head.copyOfRange(SECTOR_SIZE.toInt(), head.size)))
            throw InvalidImageException("Invalid GPT header")

        val hdrSize = hdr.getInt(12)
        if (hdrSize !in GPT_HEADER_SIZE..SECTOR_SIZE)
            throw InvalidImageException("Invalid GPT header size")
        val hdrCrc = hdr.getInt(16)
        val hdrBytes = head.copyOfRange(SECTOR_SIZE.toInt(), SECTOR_SIZE.toInt() + hdrSize)
        hdrBytes.fill(0, 16, 20)
        if (crc32(hdrBytes) != hdrCrc)
            throw InvalidImageException("GPT header checksum mismatch")

        val entryLba = hdr.getLong(72)
        val entryNum = hdr.getInt(80)
        val entrySize = hdr.getInt(84)
        val entryCrc = hdr.getInt(88)
        if (entryLba < 2 || entryNum <= 0 || entrySize < GPT_ENTRY_SIZE || entryNum > 1024)
            throw InvalidImageException("Invalid GPT partition entries")

        // Read partition entries
        copy(entryLba * SECTOR_SIZE - position)
        val entries = ByteArray(entryNum * entrySize)
        readFully(entries)
        output.write(entries)
        position += entries.size
        if (crc32(entries) != entryCrc)
            throw InvalidImageException("GPT partition entries checksum mismatch")

        val buf = ByteBuffer.wrap(entries).order(ByteOrder.LITTLE_ENDIAN)
        val parts = mutableListOf<Partition>()
        for (i in 0 until entryNum) {
            val base = i * entrySize
            // Skip unused entries (type GUID is all zeros)
            if (buf.getLong(base) == 0L && buf.getLong(base + 8) == 0L)
                continue
            val firstLba = buf.getLong(base + 32)
            val lastLba = buf.getLong(base + 40)
            val name = String(entries, base + 56, 72, Charsets.UTF_16LE).substringBefore('\u0000')
            if (lastLba < firstLba)
                throw InvalidImageException("Invalid partition: $name")
            parts.add(Partition(name, firstLba, lastLba))
        }
        parts.sortBy { it.offset }

        // Partitions must not overlap with each other or the partition table
        var end = position
        for (part in parts) {
            if (part.offset < end)
                throw InvalidImageException("Overlapping partition: ${part.name}")
            end = part.offset + part.size
        }
        if (parts.none { it.name == INIT_BOOT })
            throw InvalidImageException("$INIT_BOOT not found")
        if (parts.none { it.name == VENDOR_BOOT })
            throw InvalidImageException("$VENDOR_BOOT not found")
        return parts
    }

    private fun crc32(bytes: ByteArray) = CRC32().run {
        update(bytes)
        value.toInt()
    }

    private fun readFully(b: ByteArray) {
        var off = 0
        while (off < b.size) {
            val n = input.read(b, off, b.size - off)
            if (n < 0) throw EOFException()
            off += n
        }
    }

    // Copy at most len bytes from input to output, stop at EOF
    private suspend fun copy(len: Long) = transfer(len, output, allowEof = true)

    // Copy exactly len bytes from input to out
    private suspend fun extract(len: Long, out: OutputStream) = transfer(len, out, allowEof = false)

    private suspend fun transfer(len: Long, out: OutputStream, allowEof: Boolean) {
        withContext(Dispatchers.IO) {
            var remain = len
            while (remain > 0) {
                ensureActive()
                val n = input.read(buffer, 0, minOf(remain, BUFFER_SIZE.toLong()).toInt())
                if (n < 0) {
                    if (allowEof) break
                    throw EOFException("Unexpected end of recovery image")
                }
                out.write(buffer, 0, n)
                remain -= n
                position += n
            }
        }
    }

    companion object {
        private const val SECTOR_SIZE = 512L
        private const val GPT_HEADER_SIZE = 92
        private const val GPT_ENTRY_SIZE = 128
        private const val BUFFER_SIZE = 1024 * 1024
        private val GPT_MAGIC = "EFI PART".toByteArray()

        private const val INIT_BOOT = "init_boot_a"
        private const val VENDOR_BOOT = "vendor_boot_b"

        /**
         * Check whether [lba1] (the second sector of the input) is a GPT header.
         */
        fun isGpt(lba1: ByteArray) = lba1.size >= GPT_MAGIC.size &&
            lba1.copyOf(GPT_MAGIC.size).contentEquals(GPT_MAGIC)
    }
}
