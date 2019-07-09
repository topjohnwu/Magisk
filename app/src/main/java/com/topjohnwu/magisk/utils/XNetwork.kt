package com.topjohnwu.magisk.utils

import android.content.Context
import okhttp3.ResponseBody
import java.io.File
import java.io.InputStream
import java.io.OutputStream

inline fun ResponseBody.writeToCachedFile(
    context: Context,
    fileName: String,
    progress: (Long) -> Unit = {}
): File {
    val file = File(context.cacheDir, fileName)
    withStreams(byteStream(), file.outputStream()) { inStream, outStream ->
        inStream.copyToWithProgress(outStream, progress)
    }
    return file
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