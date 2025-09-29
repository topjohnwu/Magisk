package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.utils.HttpFileChannel
import okio.buffer
import okio.inflate
import okio.sink
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipFile
import org.apache.commons.compress.archivers.zip.ZipMethod
import java.io.File
import java.io.IOException
import java.nio.channels.FileChannel
import java.nio.file.StandardOpenOption

class ExtractImage(
    private val url: String,
    private val console: MutableList<String>,
    private val logs: MutableList<String>,
) {
    @Throws(IOException::class)
    fun start(outFile: File) {
        logs.add("Downloading from: $url")

        val channel = HttpFileChannel(ServiceLocator.okhttp, url)
        ZipFile.builder()
            .setSeekableByteChannel(channel)
            .setIgnoreLocalFileHeader(true)
            .get().use { zipFile ->
                val payload = zipFile.getEntry("payload.bin")
                if (payload != null) {
                    console.add("- Processing as OTA package")

                    zipFile.getEntry("META-INF/com/android/metadata")?.let { entry ->
                        zipFile.getInputStream(entry).use {
                            val meta = it.bufferedReader().readText()
                            logs.add(meta)

                            console.add("- OTA metadata:")
                            meta.lines().forEach { line ->
                                if (line.startsWith("post-")) {
                                    console.add("  ${line.substringAfter('-')}")
                                }
                            }
                        }
                    }
                    zipFile.getRawInputStream(payload)
                    extractFromOTAPackage(payload, channel, outFile)
                } else {
                    extractFromFactoryImage(zipFile, channel, outFile)
                }
            }
    }

    @Throws(IOException::class)
    private fun extractFromOTAPackage(
        payload: ZipArchiveEntry,
        channel: HttpFileChannel,
        outFile: File,
    ) {
        if (payload.method != ZipMethod.STORED.code) {
            throw IOException("payload.bin is compressed, expected STORED method")
        }

        channel.slice(payload.dataOffset, payload.size).use { payloadChannel ->
            Payload(payloadChannel).extract(outFile, { console.add(it) }, { logs.add(it) })
        }
    }

    @Throws(IOException::class)
    private fun extractFromFactoryImage(zipFile: ZipFile, channel: HttpFileChannel, outFile: File) {
        console.add("- Processing as factory image package")

        findBootImageZipEntry(zipFile)?.let { entry ->
            return extractImageFile(zipFile, entry, channel, outFile)
        }

        val imageZipEntry = zipFile.entries.asSequence().find { entry ->
            val fileName = entry.name.substringAfterLast('/')
            fileName.startsWith("image-") && fileName.endsWith(".zip")
        }
        if (imageZipEntry != null) {
            zipFile.getRawInputStream(imageZipEntry)
            return extractFromInnerImageZip(imageZipEntry, channel, outFile)
        }

        throw IOException("inner image ZIP not found in factory image package")
    }

    private fun findBootImageZipEntry(zipFile: ZipFile): ZipArchiveEntry? {
        return zipFile.entries.asSequence().find { it.name == "init_boot.img" }
            ?: zipFile.entries.asSequence().find { it.name == "boot.img" }
    }

    @Throws(IOException::class)
    private fun extractFromInnerImageZip(
        entry: ZipArchiveEntry,
        channel: HttpFileChannel,
        outFile: File
    ) {
        logs.add("Found inner image ZIP: ${entry.name}")

        if (entry.method != ZipMethod.STORED.code) {
            throw IOException("image ZIP is compressed, expected STORED method")
        }

        channel.slice(entry.dataOffset, entry.size).use { innerZipChannel ->
            ZipFile.builder()
                .setSeekableByteChannel(innerZipChannel)
                .setIgnoreLocalFileHeader(true)
                .get().use { innerZipFile ->
                    val targetEntry = findBootImageZipEntry(innerZipFile)
                        ?: throw IOException("boot image not found in inner image ZIP")
                    return extractImageFile(innerZipFile, targetEntry, innerZipChannel, outFile)
                }
        }
    }

    @Throws(IOException::class)
    private fun extractImageFile(
        zipFile: ZipFile,
        entry: ZipArchiveEntry,
        channel: HttpFileChannel,
        outFile: File,
    ) {
        console.add("- Found boot image entry: ${entry.name} (${entry.size} bytes)")
        console.add("- Downloading")

        zipFile.getRawInputStream(entry)
        when (entry.method) {
            ZipMethod.STORED.code -> {
                FileChannel.open(
                    outFile.toPath(),
                    StandardOpenOption.CREATE,
                    StandardOpenOption.WRITE,
                    StandardOpenOption.READ,
                    StandardOpenOption.TRUNCATE_EXISTING
                ).use { fileChannel ->
                    val mapped = fileChannel.map(FileChannel.MapMode.READ_WRITE, 0, entry.size)
                    val sourceChannel = channel.slice(entry.dataOffset, entry.size)
                    sourceChannel.read(mapped)
                }
            }

            ZipMethod.DEFLATED.code -> {
                channel.streamRead(entry.dataOffset, entry.size).inflate().use { source ->
                    outFile.sink().buffer().use { sink ->
                        sink.writeAll(source)
                    }
                }
            }

            else -> throw IOException("unsupported method: ${entry.method}")
        }
    }
}
