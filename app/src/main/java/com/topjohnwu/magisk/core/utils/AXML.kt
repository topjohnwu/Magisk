package com.topjohnwu.magisk.core.utils

import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder.LITTLE_ENDIAN
import java.nio.charset.Charset

class AXML(b: ByteArray) {

    var bytes = b
        private set

    companion object {
        private const val CHUNK_SIZE_OFF = 4
        private const val STRING_INDICES_OFF = 7 * 4
        private val UTF_16LE = Charset.forName("UTF-16LE")
    }

    /**
     * String pool header:
     * 0:  0x1C0001
     * 1:  chunk size
     * 2:  number of strings
     * 3:  number of styles (assert as 0)
     * 4:  flags
     * 5:  offset to string data
     * 6:  offset to style data (assert as 0)
     *
     * Followed by an array of uint32_t with size = number of strings
     * Each entry points to an offset into the string data
     */
    fun patchStrings(patchFn: (Array<String>) -> Unit): Boolean {
        val buffer = ByteBuffer.wrap(bytes).order(LITTLE_ENDIAN)

        fun findStringPool(): Int {
            var offset = 8
            while (offset < bytes.size) {
                if (buffer.getInt(offset) == 0x1C0001)
                    return offset
                offset += buffer.getInt(offset + CHUNK_SIZE_OFF)
            }
            return -1
        }

        val start = findStringPool()
        if (start < 0)
            return false

        // Read header
        buffer.position(start + 4)
        val intBuf = buffer.asIntBuffer()
        val size = intBuf.get()
        val count = intBuf.get()
        intBuf.get()
        intBuf.get()
        val dataOff = start + intBuf.get()
        intBuf.get()

        val strList = ArrayList<String>(count)
        // Collect all strings in the pool
        for (i in 0 until count) {
            val off = dataOff + intBuf.get()
            val len = buffer.getShort(off)
            strList.add(String(bytes, off + 2, len * 2, UTF_16LE))
        }

        val strArr = strList.toTypedArray()
        patchFn(strArr)

        // Write everything before string data, will patch values later
        val baos = RawByteStream()
        baos.write(bytes, 0, dataOff)

        // Write string data
        val offList = IntArray(count)
        for (i in 0 until count) {
            offList[i] = baos.size() - dataOff
            val str = strArr[i]
            baos.write(str.length.toShortBytes())
            baos.write(str.toByteArray(UTF_16LE))
            // Null terminate
            baos.write(0)
            baos.write(0)
        }
        baos.align()

        val sizeDiff = baos.size() - start - size
        val newBuffer = ByteBuffer.wrap(baos.buf).order(LITTLE_ENDIAN)

        // Patch XML size
        newBuffer.putInt(CHUNK_SIZE_OFF, buffer.getInt(CHUNK_SIZE_OFF) + sizeDiff)
        // Patch string pool size
        newBuffer.putInt(start + CHUNK_SIZE_OFF, size + sizeDiff)
        // Patch index table
        newBuffer.position(start + STRING_INDICES_OFF)
        val newIntBuf = newBuffer.asIntBuffer()
        offList.forEach { newIntBuf.put(it) }

        // Write the rest of the chunks
        val nextOff = start + size
        baos.write(bytes, nextOff, bytes.size - nextOff)

        bytes = baos.toByteArray()
        return true
    }

    private fun Int.toShortBytes(): ByteArray {
        val b = ByteBuffer.allocate(2).order(LITTLE_ENDIAN)
        b.putShort(this.toShort())
        return b.array()
    }

    private class RawByteStream : ByteArrayOutputStream() {
        val buf: ByteArray get() = buf

        fun align(alignment: Int = 4) {
            val newCount = (count + alignment - 1) / alignment * alignment
            for (i in 0 until (newCount - count))
                write(0)
        }
    }
}
