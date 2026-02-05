package com.topjohnwu.magisk.core.tasks

import chromeos_update_engine.UpdateMetadata.DeltaArchiveManifest
import chromeos_update_engine.UpdateMetadata.InstallOperation
import chromeos_update_engine.UpdateMetadata.PartitionUpdate
import com.topjohnwu.magisk.core.utils.DataSourceChannel
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream
import org.apache.commons.compress.compressors.xz.XZCompressorInputStream
import java.io.File
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.FileChannel
import java.nio.file.StandardOpenOption
import java.security.MessageDigest

class Payload(private val channel: DataSourceChannel) {
    private val manifest: DeltaArchiveManifest
    private var dataBase = 0L

    init {
        manifest = readPayloadHeader()
    }

    @Throws(IOException::class)
    fun extract(outputFile: File, console: (String) -> Unit, logger: (String) -> Unit) {
        val partition = findPartition()
        console("- Found partition ${partition.partitionName}")

        val actualHash = extractPartition(outputFile, partition, console)

        if (!partition.newPartitionInfo.hasHash()) {
            logger("Hash verification skipped")
            return
        }

        fun toHex(bytes: ByteArray) = bytes.joinToString("") { "%02x".format(it) }

        val expectedHash = partition.newPartitionInfo.hash.toByteArray()
        if (!expectedHash.contentEquals(actualHash)) {
            throw IOException(
                "Hash mismatch, expected ${toHex(expectedHash)}, but got ${toHex(actualHash)}"
            )
        }
        logger("Hash verification passed")
    }

    @Throws(IOException::class)
    private fun readPayloadHeader(): DeltaArchiveManifest {
        // Read magic
        val magicBuffer = ByteBuffer.allocate(4)
        channel.read(magicBuffer)
        magicBuffer.flip()
        val magic = String(magicBuffer.array())
        if (magic != "CrAU") {
            throw IOException("Invalid payload: invalid magic")
        }

        // Read version
        val versionBuffer = ByteBuffer.allocate(8).order(ByteOrder.BIG_ENDIAN)
        channel.read(versionBuffer)
        versionBuffer.flip()
        val version = versionBuffer.long
        if (version != 2L) {
            throw IOException("Invalid payload: unsupported version: $version")
        }

        // Read manifest length
        val manifestLenBuffer = ByteBuffer.allocate(8).order(ByteOrder.BIG_ENDIAN)
        channel.read(manifestLenBuffer)
        manifestLenBuffer.flip()
        val manifestLen = manifestLenBuffer.long.toInt()
        if (manifestLen == 0) {
            throw IOException("Invalid payload: manifest length is zero")
        }

        // Read manifest signature length
        val manifestSigLenBuffer = ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN)
        channel.read(manifestSigLenBuffer)
        manifestSigLenBuffer.flip()
        val manifestSigLen = manifestSigLenBuffer.int
        if (manifestSigLen == 0) {
            throw IOException("Invalid payload: manifest signature length is zero")
        }

        // Read manifest
        val manifestBuffer = ByteBuffer.allocate(manifestLen)
        channel.read(manifestBuffer)
        manifestBuffer.flip()
        val manifest = DeltaArchiveManifest.parseFrom(manifestBuffer.array())

        // Skip manifest signature
        channel.position(channel.position() + manifestSigLen)

        dataBase = channel.position()

        return manifest
    }

    @Throws(IOException::class)
    private fun findPartition(): PartitionUpdate {
        return manifest.partitionsList.find { it.partitionName == "init_boot" }
            ?: manifest.partitionsList.find { it.partitionName == "boot" }
            ?: throw IOException("boot partition not found in payload")
    }

    @Throws(IOException::class)
    private fun extractPartition(
        outputFile: File,
        partition: PartitionUpdate,
        console: (String) -> Unit,
    ): ByteArray {
        FileChannel.open(
            outputFile.toPath(),
            StandardOpenOption.CREATE,
            StandardOpenOption.WRITE,
            StandardOpenOption.READ,
            StandardOpenOption.TRUNCATE_EXISTING
        ).use { outChannel ->
            val size = partition.newPartitionInfo.size
            outChannel.write(ByteBuffer.allocate(1), size - 1)

            val count = partition.operationsCount
            partition.operationsList.forEachIndexed { index, operation ->
                if (index % 5 == 0 || index == count - 1) {
                    console("- Downloading ${index + 1}/$count")
                }
                processOperation(outChannel, operation)
            }

            val digest = MessageDigest.getInstance("SHA-256")
            val buffer = outChannel.map(FileChannel.MapMode.READ_WRITE, 0, size)
            digest.update(buffer)
            return digest.digest()
        }
    }

    @Throws(IOException::class)
    private fun processOperation(outChannel: FileChannel, operation: InstallOperation) {
        val dataType = operation.getType()
        if (dataType == InstallOperation.Type.ZERO) {
            return
        }

        val dataBuffer = ByteBuffer.allocate(operation.getDataLength().toInt())
        channel.read(dataBuffer, dataBase + operation.getDataOffset())
        dataBuffer.flip()

        val dstExtent = operation.getDstExtents(0)
        val outOffset = dstExtent.getStartBlock() * manifest.getBlockSize()

        when (dataType) {
            InstallOperation.Type.REPLACE -> {
                outChannel.write(dataBuffer, outOffset)
            }

            InstallOperation.Type.REPLACE_BZ, InstallOperation.Type.REPLACE_XZ -> {
                val inputStream = dataBuffer.array().inputStream()
                if (dataType == InstallOperation.Type.REPLACE_BZ) {
                    BZip2CompressorInputStream(inputStream)
                } else {
                    XZCompressorInputStream(inputStream)
                }.use { decompressor ->
                    val bytes = ByteArray(8192)
                    var bytesRead: Int
                    var bytesWritten = 0
                    while (decompressor.read(bytes).also { bytesRead = it } != -1) {
                        val buffer = ByteBuffer.wrap(bytes, 0, bytesRead)
                        bytesWritten += outChannel.write(buffer, outOffset + bytesWritten)
                    }
                }
            }

            else -> throw IOException("Unsupported operation type: $dataType")
        }
    }
}
