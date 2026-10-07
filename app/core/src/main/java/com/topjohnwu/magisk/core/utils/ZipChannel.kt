package com.topjohnwu.magisk.core.utils

import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipFile
import org.apache.commons.compress.archivers.zip.ZipMethod
import java.io.Closeable
import java.io.IOException
import java.io.InputStream
import java.util.zip.Inflater
import java.util.zip.InflaterInputStream

/**
 * A zip archive backed by a [DataChannel].
 *
 * [entries] lists all files in the archive. The content of an entry can be accessed with:
 * - [open]: a [DataChannel] supporting random access. Only STORED entries are supported.
 * - [openStream]: a sequential [InputStream]. Both STORED and DEFLATED entries are supported.
 *
 * The archive does not take ownership of the source channel. Both the source channel
 * and the archive must remain open while using channels or streams of its entries.
 */
class ZipChannel(private val source: DataChannel) : Closeable {

    // Use a slice so that closing the zip file does not close the source channel
    private val zip = ZipFile.builder()
        .setSeekableByteChannel(source.slice(0, source.size()))
        .setIgnoreLocalFileHeader(true)
        .get()

    /** All files (excluding directories) in the archive. */
    val entries: List<ZipArchiveEntry> by lazy {
        zip.entries.toList().filter { !it.isDirectory }
    }

    fun getEntry(name: String): ZipArchiveEntry? = zip.getEntry(name)

    fun find(predicate: (ZipArchiveEntry) -> Boolean) = entries.find(predicate)

    /**
     * Get a [DataChannel] of the content of [entry], which must be STORED.
     * The caller is responsible for closing the returned channel.
     */
    @Throws(IOException::class)
    fun open(entry: ZipArchiveEntry): DataChannel {
        if (entry.method != ZipMethod.STORED.code) {
            throw IOException("${entry.name} is compressed, expected STORED method")
        }
        if (entry.size <= 0) {
            throw IOException("Empty entry: ${entry.name}")
        }
        return source.slice(dataOffset(entry), entry.size)
    }

    /**
     * Get an [InputStream] of the decompressed content of [entry].
     * The caller is responsible for closing the returned stream.
     */
    @Throws(IOException::class)
    fun openStream(entry: ZipArchiveEntry): InputStream {
        return when (entry.method) {
            ZipMethod.STORED.code -> source.stream(dataOffset(entry), entry.size)
            ZipMethod.DEFLATED.code -> {
                val inflater = Inflater(true)
                val raw = source.stream(dataOffset(entry), entry.compressedSize)
                object : InflaterInputStream(raw, inflater, BUFFER_SIZE) {
                    override fun close() {
                        try {
                            super.close()
                        } finally {
                            inflater.end()
                        }
                    }
                }
            }
            else -> throw IOException("Unsupported compression method ${entry.method}: ${entry.name}")
        }
    }

    // Resolve the data offset, which requires parsing the local file header
    private fun dataOffset(entry: ZipArchiveEntry): Long {
        zip.getRawInputStream(entry).close()
        return entry.dataOffset
    }

    override fun close() = zip.close()

    companion object {
        private const val BUFFER_SIZE = 64 * 1024
    }
}
