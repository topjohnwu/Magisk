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

fun <T> MutableList<T>.update(newList: List<T>) {
    clear()
    addAll(newList)
}

operator fun <E> SparseArrayCompat<E>.set(key: Int, value: E) {
    put(key, value)
}

fun <T> MutableList<T>.synchronized() = Collections.synchronizedList(this)
fun <T> MutableSet<T>.synchronized() = Collections.synchronizedSet(this)
fun <K, V> MutableMap<K, V>.synchronized() = Collections.synchronizedMap(this)

fun SimpleDateFormat.parseOrNull(date: String) =
    runCatching { parse(date) }.onFailure { Timber.e(it) }.getOrNull()

fun Class<*>.reflectField(name: String): Field =
    getDeclaredField(name).apply { isAccessible = true }
