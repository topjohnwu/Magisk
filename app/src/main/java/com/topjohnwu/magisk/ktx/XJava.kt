package com.topjohnwu.magisk.ktx

import androidx.collection.SparseArrayCompat
import timber.log.Timber
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.lang.reflect.Field
import java.text.SimpleDateFormat
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

inline fun ZipInputStream.forEach(callback: (ZipEntry) -> Unit) {
    var entry: ZipEntry? = nextEntry
    while (entry != null) {
        callback(entry)
        entry = nextEntry
    }
}

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

fun InputStream.copyAndClose(out: OutputStream) = withStreams(this, out) { i, o -> i.copyTo(o) }

fun InputStream.writeTo(file: File) = copyAndClose(file.outputStream())

operator fun <E> SparseArrayCompat<E>.set(key: Int, value: E) {
    put(key, value)
}

fun <T> MutableList<T>.synchronized(): MutableList<T> = Collections.synchronizedList(this)

fun <T> MutableSet<T>.synchronized(): MutableSet<T> = Collections.synchronizedSet(this)

fun <K, V> MutableMap<K, V>.synchronized(): MutableMap<K, V> = Collections.synchronizedMap(this)

fun SimpleDateFormat.parseOrNull(date: String) =
    runCatching { parse(date) }.onFailure { Timber.e(it) }.getOrNull()

fun Class<*>.reflectField(name: String): Field =
    getDeclaredField(name).apply { isAccessible = true }
