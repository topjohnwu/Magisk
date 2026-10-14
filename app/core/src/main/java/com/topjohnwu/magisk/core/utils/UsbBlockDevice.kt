package com.topjohnwu.magisk.core.utils

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbConstants
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbEndpoint
import android.hardware.usb.UsbInterface
import android.hardware.usb.UsbManager
import android.os.Build
import androidx.core.content.ContextCompat
import kotlinx.coroutines.delay
import kotlinx.coroutines.suspendCancellableCoroutine
import java.io.IOException
import java.io.OutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlin.coroutines.resume

object UsbBlockDevice {

    private const val SCSI_SUBCLASS = 0x06
    private const val BOT_PROTOCOL = 0x50

    data class DeviceInfo(
        val deviceName: String,
        val label: String,
        val vendorId: Int,
        val productId: Int,
    )

    private class BotInterface(
        val usbInterface: UsbInterface,
        val inEndpoint: UsbEndpoint,
        val outEndpoint: UsbEndpoint,
    )

    fun listDevices(context: Context): List<DeviceInfo> {
        val usbManager = context.getSystemService(UsbManager::class.java) ?: return emptyList()
        return usbManager.deviceList.values.mapNotNull { device ->
            findBotInterface(device) ?: return@mapNotNull null
            val manufacturer = device.manufacturerName?.trim().orEmpty()
            val product = device.productName?.trim().orEmpty()
            val label = when {
                manufacturer.isNotEmpty() && product.isNotEmpty() -> {
                    if (product.startsWith(manufacturer, ignoreCase = true)) product
                    else "$manufacturer $product"
                }
                product.isNotEmpty() -> product
                manufacturer.isNotEmpty() -> manufacturer
                else -> device.deviceName
            }
            DeviceInfo(
                deviceName = device.deviceName,
                label = label,
                vendorId = device.vendorId,
                productId = device.productId,
            )
        }
    }

    suspend fun requestPermission(context: Context, deviceName: String): Boolean {
        val usbManager = context.getSystemService(UsbManager::class.java) ?: return false
        val device = usbManager.deviceList[deviceName] ?: return false
        if (usbManager.hasPermission(device)) return true

        val action = "${context.packageName}.USB_PERMISSION"
        return suspendCancellableCoroutine { cont ->
            val receiver = object : BroadcastReceiver() {
                override fun onReceive(ctx: Context, intent: Intent) {
                    if (intent.action != action) return
                    runCatching { context.unregisterReceiver(this) }
                    if (cont.isActive) {
                        cont.resume(usbManager.hasPermission(device))
                    }
                }
            }
            ContextCompat.registerReceiver(
                context,
                receiver,
                IntentFilter(action),
                ContextCompat.RECEIVER_EXPORTED
            )
            cont.invokeOnCancellation {
                runCatching { context.unregisterReceiver(receiver) }
            }
            val flags = PendingIntent.FLAG_UPDATE_CURRENT or
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) PendingIntent.FLAG_MUTABLE else 0
            val pendingIntent = PendingIntent.getBroadcast(
                context,
                0,
                Intent(action).setPackage(context.packageName),
                flags
            )
            usbManager.requestPermission(device, pendingIntent)
        }
    }

    @Throws(IOException::class)
    suspend fun open(
        context: Context,
        deviceName: String,
        requiredSize: Long,
        console: MutableList<String>,
        logs: MutableList<String>,
    ): OutputStream {
        val usbManager = context.getSystemService(UsbManager::class.java)
            ?: throw IOException("UsbManager unavailable")
        val device = usbManager.deviceList[deviceName]
            ?: throw IOException("USB device disconnected: $deviceName")
        if (!usbManager.hasPermission(device)) {
            throw IOException("Permission denied for USB device: $deviceName")
        }
        val bot = findBotInterface(device)
            ?: throw IOException("USB device does not support Mass Storage Bulk-Only Transport")

        val connection = usbManager.openDevice(device)
            ?: throw IOException("Failed to open USB device: $deviceName")
        var success = false
        try {
            if (!connection.claimInterface(bot.usbInterface, true)) {
                throw IOException("Failed to claim USB Mass Storage interface")
            }
            val stream = UsbBotOutputStream(connection, bot.usbInterface, bot.inEndpoint, bot.outEndpoint, logs)
            stream.initialize(requiredSize, console)
            success = true
            return stream
        } finally {
            if (!success) {
                runCatching { connection.releaseInterface(bot.usbInterface) }
                runCatching { connection.close() }
            }
        }
    }

    private fun findBotInterface(device: UsbDevice): BotInterface? {
        for (i in 0 until device.interfaceCount) {
            val intf = device.getInterface(i)
            if (intf.interfaceClass != UsbConstants.USB_CLASS_MASS_STORAGE ||
                intf.interfaceSubclass != SCSI_SUBCLASS ||
                intf.interfaceProtocol != BOT_PROTOCOL
            ) continue

            var inEp: UsbEndpoint? = null
            var outEp: UsbEndpoint? = null
            for (j in 0 until intf.endpointCount) {
                val ep = intf.getEndpoint(j)
                if (ep.type == UsbConstants.USB_ENDPOINT_XFER_BULK) {
                    if (ep.direction == UsbConstants.USB_DIR_IN) {
                        inEp = ep
                    } else if (ep.direction == UsbConstants.USB_DIR_OUT) {
                        outEp = ep
                    }
                }
            }
            if (inEp != null && outEp != null) {
                return BotInterface(
                    usbInterface = intf,
                    inEndpoint = inEp,
                    outEndpoint = outEp,
                )
            }
        }
        return null
    }
}

private class UsbBotOutputStream(
    private val connection: UsbDeviceConnection,
    private val usbInterface: UsbInterface,
    private val inEndpoint: UsbEndpoint,
    private val outEndpoint: UsbEndpoint,
    private val logs: MutableList<String>,
) : OutputStream() {

    private var tag = 1
    private var blockSize = 512
    private var blockCount = 0L
    private var currentLba = 0L
    private var closed = false
    private var failed = false
    private var synced = false

    private val buffer = ByteArray(BUFFER_SIZE)
    private var bufPos = 0

    private val cbwBytes = ByteArray(CBW_SIZE)
    private val cbwBuf = ByteBuffer.wrap(cbwBytes).order(ByteOrder.LITTLE_ENDIAN)
    private val cswBytes = ByteArray(CSW_SIZE)
    private val cswBuf = ByteBuffer.wrap(cswBytes).order(ByteOrder.LITTLE_ENDIAN)

    @Throws(IOException::class)
    suspend fun initialize(requiredSize: Long, console: MutableList<String>) {
        // 1. SCSI INQUIRY (0x12)
        val inquiry = ByteArray(36)
        val inquiryStatus = executeScsiIn(
            cdb = byteArrayOf(0x12, 0, 0, 0, inquiry.size.toByte(), 0),
            data = inquiry,
            length = inquiry.size,
            timeout = CONTROL_TIMEOUT
        )
        if (inquiryStatus != 0) {
            throw IOException("SCSI INQUIRY failed (status=$inquiryStatus)")
        }
        val peripheralType = inquiry[0].toInt() and 0x1F
        if (peripheralType != 0x00) {
            throw IOException("Unsupported SCSI peripheral type: 0x${peripheralType.toString(16)}")
        }
        val vendor = String(inquiry, 8, 8, Charsets.US_ASCII).trim()
        val product = String(inquiry, 16, 16, Charsets.US_ASCII).trim()

        // 2. SCSI TEST UNIT READY (0x00) + REQUEST SENSE (0x03) retry loop
        var ready = false
        for (attempt in 0 until 10) {
            val turStatus = executeScsiOut(
                cdb = byteArrayOf(0x00, 0, 0, 0, 0, 0),
                data = buffer,
                length = 0,
                timeout = CONTROL_TIMEOUT
            )
            if (turStatus == 0) {
                ready = true
                break
            }
            requestSense()
            delay(100)
        }
        if (!ready) {
            throw IOException("USB drive is not ready")
        }

        // 3. SCSI READ CAPACITY (10) (0x25) / READ CAPACITY (16) (0x9E/0x10)
        val cap10 = ByteArray(8)
        val cap10Status = executeScsiIn(
            cdb = byteArrayOf(0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0),
            data = cap10,
            length = cap10.size,
            timeout = CONTROL_TIMEOUT
        )
        if (cap10Status != 0) {
            requestSense()
            throw IOException("SCSI READ CAPACITY(10) failed (status=$cap10Status)")
        }
        val cap10Buf = ByteBuffer.wrap(cap10).order(ByteOrder.BIG_ENDIAN)
        val lastLba32 = cap10Buf.int.toLong() and 0xFFFFFFFFL
        val blkLen10 = cap10Buf.int

        if (lastLba32 == 0xFFFFFFFFL) {
            val cap16 = ByteArray(32)
            val cap16Status = executeScsiIn(
                cdb = byteArrayOf(
                    0x9E.toByte(), 0x10,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, cap16.size.toByte(),
                    0, 0
                ),
                data = cap16,
                length = cap16.size,
                timeout = CONTROL_TIMEOUT
            )
            if (cap16Status != 0) {
                requestSense()
                throw IOException("SCSI READ CAPACITY(16) failed (status=$cap16Status)")
            }
            val cap16Buf = ByteBuffer.wrap(cap16).order(ByteOrder.BIG_ENDIAN)
            blockCount = cap16Buf.long + 1L
            blockSize = cap16Buf.int
        } else {
            blockCount = lastLba32 + 1L
            blockSize = blkLen10
        }

        if (blockSize <= 0 || blockSize % 512 != 0 || BUFFER_SIZE % blockSize != 0) {
            throw IOException("Unsupported USB logical block size: $blockSize")
        }

        val capacityBytes = blockCount * blockSize
        val capacityMb = capacityBytes / (1024 * 1024)
        console.add("- Target USB: $vendor $product ($capacityMb MB)")
        logs.add("USB device: $vendor $product, blocks=$blockCount, blockSize=$blockSize, capacity=$capacityBytes")

        if (requiredSize > 0 && capacityBytes < requiredSize) {
            val reqMb = (requiredSize + 1024 * 1024 - 1) / (1024 * 1024)
            throw IOException("USB drive is too small (${capacityMb} MB < ${reqMb} MB required)")
        }
    }

    override fun write(b: Int) {
        if (closed) throw IOException("Stream closed")
        buffer[bufPos++] = b.toByte()
        if (bufPos == BUFFER_SIZE) {
            flushBuffer(BUFFER_SIZE)
        }
    }

    override fun write(b: ByteArray, off: Int, len: Int) {
        if (closed) throw IOException("Stream closed")
        var offset = off
        var remaining = len
        while (remaining > 0) {
            val copyLen = minOf(remaining, BUFFER_SIZE - bufPos)
            System.arraycopy(b, offset, buffer, bufPos, copyLen)
            bufPos += copyLen
            offset += copyLen
            remaining -= copyLen
            if (bufPos == BUFFER_SIZE) {
                flushBuffer(BUFFER_SIZE)
            }
        }
    }

    override fun flush() {
        if (closed) throw IOException("Stream closed")
        try {
            if (bufPos > 0) {
                val rem = bufPos % blockSize
                val alignedLen = if (rem == 0) bufPos else {
                    val pad = blockSize - rem
                    buffer.fill(0, bufPos, bufPos + pad)
                    bufPos + pad
                }
                flushBuffer(alignedLen)
            }
            syncCache()
            synced = true
        } catch (e: IOException) {
            failed = true
            throw e
        }
    }

    override fun close() {
        if (closed) return
        try {
            if (!failed && !synced) {
                flush()
            }
        } finally {
            closed = true
            runCatching { connection.releaseInterface(usbInterface) }
            runCatching { connection.close() }
        }
    }

    @Throws(IOException::class)
    private fun flushBuffer(length: Int) {
        try {
            val blocks = length / blockSize
            if (currentLba + blocks > blockCount) {
                throw IOException("Write exceeds USB drive capacity at LBA $currentLba")
            }
            val cdb = if (currentLba + blocks <= 0xFFFFFFFFL && blocks <= 0xFFFF) {
                val lba = currentLba.toInt()
                byteArrayOf(
                    0x2A, // WRITE(10)
                    0x00,
                    (lba ushr 24).toByte(),
                    (lba ushr 16).toByte(),
                    (lba ushr 8).toByte(),
                    lba.toByte(),
                    0x00,
                    (blocks ushr 8).toByte(),
                    blocks.toByte(),
                    0x00
                )
            } else {
                val cdb16 = ByteArray(16)
                val buf = ByteBuffer.wrap(cdb16).order(ByteOrder.BIG_ENDIAN)
                buf.put(0, 0x8A.toByte()) // WRITE(16)
                buf.putLong(2, currentLba)
                buf.putInt(10, blocks)
                cdb16
            }

            val status = executeScsiOut(cdb, buffer, length, WRITE_TIMEOUT)
            if (status != 0) {
                val sense = requestSense()
                throw IOException("SCSI WRITE failed at LBA $currentLba (status=$status, sense=$sense)")
            }
            currentLba += blocks
            bufPos = 0
            synced = false
        } catch (e: IOException) {
            failed = true
            throw e
        }
    }

    @Throws(IOException::class)
    private fun syncCache() {
        val status = executeScsiOut(
            cdb = byteArrayOf(0x35, 0, 0, 0, 0, 0, 0, 0, 0, 0),
            data = buffer,
            length = 0,
            timeout = WRITE_TIMEOUT
        )
        if (status != 0) {
            val sense = requestSense()
            // Ignore Illegal Request (senseKey == 0x05) on devices that do not implement SYNCHRONIZE CACHE
            if (!sense.startsWith("key=0x5")) {
                logs.add("SCSI SYNCHRONIZE CACHE(10) returned status=$status ($sense)")
            }
        }
    }

    private fun requestSense(): String {
        val sense = ByteArray(18)
        val status = runCatching {
            executeScsiIn(
                cdb = byteArrayOf(0x03, 0, 0, 0, sense.size.toByte(), 0),
                data = sense,
                length = sense.size,
                timeout = CONTROL_TIMEOUT
            )
        }.getOrDefault(-1)
        if (status != 0) return "sense_unavailable(status=$status)"
        val key = sense[2].toInt() and 0x0F
        val asc = sense[12].toInt() and 0xFF
        val ascq = sense[13].toInt() and 0xFF
        return "key=0x${key.toString(16)}, asc=0x${asc.toString(16)}, ascq=0x${ascq.toString(16)}"
    }

    @Throws(IOException::class)
    private fun executeScsiOut(cdb: ByteArray, data: ByteArray, length: Int, timeout: Int): Int {
        val cmdTag = sendCbw(cdb, length, isDataIn = false)
        if (length > 0) {
            val transferred = connection.bulkTransfer(outEndpoint, data, 0, length, timeout)
            if (transferred != length) {
                clearHalt(outEndpoint)
                throw IOException("USB bulk OUT transfer failed ($transferred / $length bytes)")
            }
        }
        return readCsw(cmdTag, timeout)
    }

    @Throws(IOException::class)
    private fun executeScsiIn(cdb: ByteArray, data: ByteArray, length: Int, timeout: Int): Int {
        val cmdTag = sendCbw(cdb, length, isDataIn = true)
        if (length > 0) {
            val transferred = connection.bulkTransfer(inEndpoint, data, 0, length, timeout)
            if (transferred < 0) {
                clearHalt(inEndpoint)
            }
        }
        return readCsw(cmdTag, timeout)
    }

    @Throws(IOException::class)
    private fun sendCbw(cdb: ByteArray, dataLength: Int, isDataIn: Boolean): Int {
        val cmdTag = tag++
        cbwBytes.fill(0)
        cbwBuf.clear()
        cbwBuf.putInt(CBW_SIGNATURE)
        cbwBuf.putInt(cmdTag)
        cbwBuf.putInt(dataLength)
        cbwBuf.put(if (isDataIn) 0x80.toByte() else 0x00)
        cbwBuf.put(0) // LUN 0
        cbwBuf.put(cdb.size.toByte())
        cbwBuf.put(cdb)

        val sent = connection.bulkTransfer(outEndpoint, cbwBytes, 0, CBW_SIZE, CONTROL_TIMEOUT)
        if (sent != CBW_SIZE) {
            throw IOException("Failed to send USB CBW ($sent / $CBW_SIZE bytes)")
        }
        return cmdTag
    }

    @Throws(IOException::class)
    private fun readCsw(expectedTag: Int, timeout: Int): Int {
        var received = connection.bulkTransfer(inEndpoint, cswBytes, 0, CSW_SIZE, timeout)
        if (received < 0) {
            clearHalt(inEndpoint)
            received = connection.bulkTransfer(inEndpoint, cswBytes, 0, CSW_SIZE, timeout)
        }
        if (received != CSW_SIZE) {
            resetRecovery()
            throw IOException("Failed to read USB CSW ($received / $CSW_SIZE bytes)")
        }
        cswBuf.clear()
        val sig = cswBuf.int
        val rcvTag = cswBuf.int
        cswBuf.int // dataResidue
        val status = cswBuf.get().toInt() and 0xFF
        if (sig != CSW_SIGNATURE || rcvTag != expectedTag) {
            resetRecovery()
            throw IOException("Invalid USB CSW (sig=0x${sig.toString(16)}, tag=$rcvTag, expected=$expectedTag)")
        }
        if (status == 2) {
            resetRecovery()
            throw IOException("USB Mass Storage Phase Error")
        }
        return status
    }

    private fun resetRecovery() {
        connection.controlTransfer(
            UsbConstants.USB_DIR_OUT or UsbConstants.USB_TYPE_CLASS or USB_RECIP_INTERFACE,
            BOT_RESET_REQUEST,
            0,
            usbInterface.id,
            null,
            0,
            CONTROL_TIMEOUT
        )
        clearHalt(inEndpoint)
        clearHalt(outEndpoint)
    }

    private fun clearHalt(endpoint: UsbEndpoint) {
        connection.controlTransfer(
            UsbConstants.USB_DIR_OUT or UsbConstants.USB_TYPE_STANDARD or USB_RECIP_ENDPOINT,
            USB_REQ_CLEAR_FEATURE,
            ENDPOINT_HALT,
            endpoint.address,
            null,
            0,
            CONTROL_TIMEOUT
        )
    }

    companion object {
        private const val BUFFER_SIZE = 128 * 1024
        private const val CBW_SIZE = 31
        private const val CSW_SIZE = 13
        private const val CBW_SIGNATURE = 0x43425355 // "USBC"
        private const val CSW_SIGNATURE = 0x53425355 // "USBS"
        private const val CONTROL_TIMEOUT = 5_000
        private const val WRITE_TIMEOUT = 30_000

        private const val USB_RECIP_INTERFACE = 0x01
        private const val USB_RECIP_ENDPOINT = 0x02
        private const val USB_REQ_CLEAR_FEATURE = 0x01
        private const val ENDPOINT_HALT = 0x00
        private const val BOT_RESET_REQUEST = 0xFF
    }
}
