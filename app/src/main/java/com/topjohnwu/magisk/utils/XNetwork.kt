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

fun Context.cachedFile(name: String) = File(cacheDir, name)

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