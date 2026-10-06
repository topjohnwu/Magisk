package com.topjohnwu.magisk.core.tasks

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.copyAll
import com.topjohnwu.magisk.core.utils.DataChannel
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.nio.ExtendedFile
import org.apache.commons.compress.archivers.tar.TarArchiveEntry
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream
import org.apache.commons.compress.compressors.lz4.FramedLZ4CompressorInputStream
import java.io.FilterInputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.nio.ByteBuffer

/**
 * Patch boot images in tar firmware packages (e.g. Samsung AP tar files).
 *
 * A single-use, two-phase processor; the caller patches the boot image in between:
 * 1. [consume]: extract boot images from the input tar into [installDir], and copy all
 *    other entries to the output tar. Returns the image to be patched.
 * 2. [finish]: write the patched image to the output tar and finalize the archive.
 *
 * [consume] closes the input channel. The caller owns the output stream and is
 * responsible for closing it.
 */
class TarProcessor(
    private val installDir: ExtendedFile,
    output: OutputStream,
    private val console: MutableList<String>,
    private val logs: MutableList<String>,
) {
    class NoBootException : IOException()

    private val tarOut = TarArchiveOutputStream(output).apply {
        setBigNumberMode(TarArchiveOutputStream.BIGNUMBER_STAR)
        setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU)
    }

    // The image to be patched, selected by consume() and written by finish()
    private lateinit var target: BootItem

    @Throws(IOException::class)
    suspend fun consume(channel: DataChannel): ExtendedFile = channel.use {
        val tarIn = TarArchiveInputStream(it.stream().buffered(1024 * 1024))
        tarIn.use { processEntries(tarIn) }
    }

    @Throws(IOException::class)
    private suspend fun processEntries(tarIn: TarArchiveInputStream): ExtendedFile {
        console.add("- Processing tar file")
        var entry: TarArchiveEntry? = tarIn.nextEntry

        fun decompressedStream(): InputStream {
            val stream = if (tarIn.currentEntry.name.endsWith(".lz4"))
                FramedLZ4CompressorInputStream(tarIn, true) else tarIn
            return NoAvailableStream(stream)
        }

        var boot: BootItem? = null
        var initBoot: BootItem? = null
        var recovery: BootItem? = null

        while (entry != null) {
            val bootItem: BootItem?
            if (entry.name.startsWith("boot.img")) {
                bootItem = BootItem(entry)
                boot = bootItem
            } else if (entry.name.startsWith("init_boot.img")) {
                bootItem = BootItem(entry)
                initBoot = bootItem
            } else if (Config.recovery && entry.name.contains("recovery.img")) {
                bootItem = BootItem(entry)
                recovery = bootItem
            } else {
                bootItem = null
            }

            if (bootItem != null) {
                console.add("-- Extracting: ${bootItem.name}")
                bootItem.file.newOutputStream().use {
                    decompressedStream().copyAll(it, 1024 * 1024)
                }
            } else if (entry.name.contains("vbmeta.img")) {
                val rawData = decompressedStream().readBytes()
                val name = entry.name.replace(".lz4", "")

                // Valid vbmeta.img should be at least 256 bytes
                if (rawData.size >= 256) {
                    // vbmeta partition exist, disable boot vbmeta patch
                    Info.patchBootVbmeta = false
                    console.add("-- Patching  : $name")

                    // Patch flags to AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED |
                    // AVB_VBMETA_IMAGE_FLAGS_VERIFICATION_DISABLED
                    ByteBuffer.wrap(rawData).putInt(120, 3)
                } else {
                    console.add("-- Copying   : $name")
                }

                // Fetch the next entry first before modifying current entry
                val vbmeta = entry
                entry = tarIn.nextEntry

                // Update entry with new information
                vbmeta.name = name
                vbmeta.size = rawData.size.toLong()

                // Write output
                tarOut.putArchiveEntry(vbmeta)
                tarOut.write(rawData)
                tarOut.closeArchiveEntry()
                continue
            } else if (entry.name.contains("userdata.img")) {
                console.add("-- Skipping  : ${entry.name}")
            } else {
                console.add("-- Copying   : ${entry.name}")
                tarOut.putArchiveEntry(entry)
                tarIn.copyAll(tarOut)
                tarOut.closeArchiveEntry()
            }
            entry = tarIn.nextEntry ?: break
        }

        // Patch priority: recovery > init_boot > boot
        target = when {
            recovery != null -> {
                if (boot != null) {
                    // Repack boot image to prevent auto restore
                    Shell.cmd(
                        "cd $installDir",
                        "chmod -R 755 .",
                        "./magiskboot unpack boot.img",
                        "./magiskboot repack boot.img",
                        "cat new-boot.img > boot.img",
                        "./magiskboot cleanup",
                        "rm -f new-boot.img",
                        "cd /"
                    ).to(console, logs).exec()
                    boot.write()
                }
                recovery
            }
            initBoot != null -> {
                boot?.write()
                initBoot
            }
            boot != null -> boot
            else -> throw NoBootException()
        }
        return target.file
    }

    @Throws(IOException::class)
    suspend fun finish(patched: ExtendedFile) {
        target.write(patched)
        tarOut.finish()
    }

    private class NoAvailableStream(s: InputStream) : FilterInputStream(s) {
        // Make sure available is never called on the actual stream and always return 0
        // to reduce max buffer size and avoid OOM
        override fun available() = 0
    }

    private inner class BootItem(private val entry: TarArchiveEntry) {
        val name = entry.name.replace(".lz4", "")
        val file: ExtendedFile = installDir.getChildFile(name)

        // Write the image to the output tar, preserving the original entry's metadata
        suspend fun write(image: ExtendedFile = file) {
            entry.name = name
            entry.size = image.length()
            image.newInputStream().use {
                console.add("-- Writing   : $name")
                tarOut.putArchiveEntry(entry)
                it.copyAll(tarOut)
                tarOut.closeArchiveEntry()
            }
        }
    }
}
