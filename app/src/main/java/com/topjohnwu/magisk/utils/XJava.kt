package com.topjohnwu.magisk.utils

import android.net.Uri
import androidx.core.net.toFile
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

fun ZipInputStream.forEach(callback: (ZipEntry) -> Unit) {
    var entry: ZipEntry? = nextEntry
    while (entry != null) {
        callback(entry)
        entry = nextEntry
    }
}

fun Uri.writeTo(file: File) = toFile().copyTo(file)

fun InputStream.writeTo(file: File) =
    withStreams(this, file.outputStream()) { reader, writer -> reader.copyTo(writer) }

inline fun <In : InputStream, Out : OutputStream> withStreams(
    inStream: In,
    outStream: Out,
    withBoth: (In, Out) -> Unit
) {
    inStream.use { reader ->
        outStream.use { writer ->
            withBoth(reader, writer)
        }
    }
}

inline fun <T, R> List<T>.firstMap(mapper: (T) -> R?): R {
    for (item: T in this) {
        return mapper(item) ?: continue
    }
    throw NoSuchElementException("Collection contains no element matching the predicate.")
}