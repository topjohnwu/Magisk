package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.magisk.core.utils.ZipChannel
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import java.io.Closeable
import java.io.IOException
import java.io.InputStream
import java.nio.ByteBuffer

/**
 * Inspect the format of a file to be patched, and locate the payload to process.
 *
 * Zip archives are inspected with [ZipChannel]; nested zips (e.g. the inner image zip
 * of factory images) are transparently inspected instead of the outer archive.
 *
 * Based on [type], the payload can be accessed with:
 * - [openChannel]: random access, required for [Type.PayloadBin] and [Type.RawFile].
 *   Payloads within zip archives must be STORED.
 * - [openStream]: sequential access, sufficient for all other types.
 *   Payloads within zip archives are decompressed transparently.
 *
 * The classifier does not take ownership of the input channel. Both the input channel
 * and the classifier must remain open while using channels or streams of the payload.
 */
class PatchFileClassifier @Throws(IOException::class) constructor(
    input: DataChannel
) : Closeable {

    enum class Type {
        /** A raw image file, e.g. boot.img */
        RawFile,
        /** A zip archive containing boot images, e.g. factory images */
        BootZip,
        /** An A/B OTA payload, e.g. payload.bin in OTA packages */
        PayloadBin,
        /** An Aluminium OS recovery image */
        RecoveryGpt,
        /** A tar archive, e.g. Samsung firmware AP tar */
        Tar,
    }

    private val zips = mutableListOf<ZipChannel>()
    private val entryNames = mutableListOf<String>()

    // The channel containing the payload: either the payload itself when entry is null,
    // or the zip archive (the last element of zips) containing the payload entry.
    private var channel = input
    private var entry: ZipArchiveEntry? = null

    val type: Type

    init {
        try {
            type = classify()
        } catch (e: Exception) {
            close()
            throw e
        }
    }

    /** Decompressed size of the payload. */
    val size: Long get() = entry?.size ?: channel.size()

    /**
     * Path of the payload within (nested) zip archives, null if the input itself is the payload.
     */
    val entryPath: String? get() = if (entryNames.isEmpty()) null else entryNames.joinToString("/")

    /**
     * Get a [DataChannel] of the payload. Payloads in zip archives must be STORED.
     * The caller is responsible for closing the returned channel.
     */
    @Throws(IOException::class)
    fun openChannel(): DataChannel {
        val entry = entry ?: return channel.slice(0, channel.size())
        return zips.last().open(entry)
    }

    /**
     * Get an [InputStream] of the payload, decompressed if required.
     * The caller is responsible for closing the returned stream.
     */
    @Throws(IOException::class)
    fun openStream(): InputStream {
        val entry = entry ?: return channel.stream()
        return zips.last().openStream(entry)
    }

    override fun close() {
        zips.forEach { runCatching { it.close() } }
        zips.clear()
    }

    private fun classify(): Type {
        repeat(MAX_ZIP_DEPTH + 1) {
            val head = ByteArray(HEAD_SIZE)
            if (channel.read(ByteBuffer.wrap(head), 0) != head.size) {
                throw IOException("Invalid input file")
            }

            when {
                head.matches(257, TAR_MAGIC) -> return Type.Tar
                RecoveryGptProcessor.isGpt(head.copyOfRange(512, 1024)) -> return Type.RecoveryGpt
                head.matches(0, PAYLOAD_MAGIC) -> return Type.PayloadBin
                !head.matches(0, ZIP_MAGIC) -> return Type.RawFile
            }

            val zip = ZipChannel(channel)
            zips.add(zip)

            zip.getEntry("payload.bin")?.let { return found(it, Type.PayloadBin) }
            findBootImage(zip)?.let { return found(it, Type.BootZip) }

            // Factory images contain the actual images in an inner zip
            val imageZip = zip.find {
                val name = it.name.substringAfterLast('/')
                name.startsWith("image-") && name.endsWith(".zip")
            }
            if (imageZip != null) {
                entryNames.add(imageZip.name)
                channel = zip.open(imageZip)
                return@repeat
            }

            zip.find { it.name.endsWith("recovery_image.bin") }?.let {
                return found(it, Type.RecoveryGpt)
            }

            throw IOException("No supported files found in zip")
        }
        throw IOException("Too many nested zip archives")
    }

    private fun found(entry: ZipArchiveEntry, type: Type): Type {
        this.entry = entry
        entryNames.add(entry.name)
        return type
    }

    private fun findBootImage(zip: ZipChannel): ZipArchiveEntry? =
        zip.find { it.name.substringAfterLast('/') == "init_boot.img" }
            ?: zip.find { it.name.substringAfterLast('/') == "boot.img" }

    private fun ByteArray.matches(offset: Int, magic: ByteArray): Boolean {
        if (offset + magic.size > size) return false
        return magic.indices.all { this[offset + it] == magic[it] }
    }

    companion object {
        private const val HEAD_SIZE = 1024
        private const val MAX_ZIP_DEPTH = 3
        private val TAR_MAGIC = "ustar".toByteArray()
        private val PAYLOAD_MAGIC = "CrAU".toByteArray()
        private val ZIP_MAGIC = "PK\u0003\u0004".toByteArray()
    }
}
