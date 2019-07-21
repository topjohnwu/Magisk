package com.topjohnwu.magisk.utils

import android.content.Context
import com.topjohnwu.superuser.internal.UiThreadHandler
import okhttp3.ResponseBody
import java.io.File
import java.io.FilterInputStream
import java.io.InputStream
import java.io.OutputStream

inline fun ResponseBody.writeToCachedFile(
    context: Context,
    fileName: String,
    progress: (Long) -> Unit = {}
): File = byteStream().writeToCachedFile(context, fileName, progress)

inline fun InputStream.writeToCachedFile(
    context: Context,
    fileName: String,
    progress: (Long) -> Unit = {}
) = context.cachedFile(fileName).apply {
    writeToFile(this, progress)
}

inline fun InputStream.writeToFile(file: File, progress: (Long) -> Unit = {}) = file.apply {
    writeTo(file.outputStream(), progress)
}

inline fun InputStream.writeTo(output: OutputStream, progress: (Long) -> Unit = {}) {
    withStreams(this, output) { inStream, outStream ->
        inStream.copyToWithProgress(outStream, progress)
    }
}

fun ResponseBody.writeToString() = string()

inline fun InputStream.copyToWithProgress(
    out: OutputStream,
    progressEmitter: (Long) -> Unit,
    bufferSize: Int = DEFAULT_BUFFER_SIZE
): Long {
    var bytesCopied: Long = 0
    val buffer = ByteArray(bufferSize)
    var bytes = read(buffer)
    while (bytes >= 0) {
        out.write(buffer, 0, bytes)
        bytesCopied += bytes
        bytes = read(buffer)
        progressEmitter(bytesCopied)
    }
    return bytesCopied
}

class ProgInputStream(base: InputStream,
                          val progressEmitter: (Long) -> Unit = {}) : FilterInputStream(base) {

    private var bytesRead : Long = 0

    override fun read(): Int {
        val b = read()
        if (b >= 0) {
            bytesRead++
            UiThreadHandler.run { progressEmitter(bytesRead) }
        }
        return b
    }

    override fun read(b: ByteArray): Int {
        return read(b, 0, b.size)
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        val sz = super.read(b, off, len)
        if (sz > 0) {
            bytesRead += sz
            UiThreadHandler.run { progressEmitter(bytesRead) }
        }
        return sz
    }
}