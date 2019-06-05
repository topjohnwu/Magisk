package com.topjohnwu.magisk.utils

import android.net.Uri
import androidx.core.net.toFile
import org.kamranzafar.jtar.TarInputStream
import org.kamranzafar.jtar.TarOutputStream
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

fun Uri.copyTo(file: File) = toFile().copyTo(file)
fun InputStream.copyTo(file: File) =
    withStreams(this, file.outputStream()) { reader, writer -> reader.copyTo(writer) }

fun File.tarInputStream() = TarInputStream(inputStream())
fun File.tarOutputStream() = TarOutputStream(this)

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