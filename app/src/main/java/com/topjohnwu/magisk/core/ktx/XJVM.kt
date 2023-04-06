package com.topjohnwu.magisk.core.ktx

import androidx.collection.SparseArrayCompat
import com.topjohnwu.magisk.core.utils.currentLocale
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flatMapMerge
import kotlinx.coroutines.flow.flow
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.lang.reflect.Field
import java.text.DateFormat
import java.text.SimpleDateFormat
import java.util.Collections
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

fun Class<*>.reflectField(name: String): Field =
    getDeclaredField(name).apply { isAccessible = true }

inline fun <T, R> Flow<T>.concurrentMap(crossinline transform: suspend (T) -> R): Flow<R> {
    return flatMapMerge { value ->
        flow { emit(transform(value)) }
    }
}

fun Long.toTime(format: DateFormat) = format.format(this).orEmpty()

// Some devices don't allow filenames containing ":"
val timeFormatStandard by lazy {
    SimpleDateFormat(
        "yyyy-MM-dd'T'HH.mm.ss",
        currentLocale
    )
}
val timeDateFormat: DateFormat by lazy {
    DateFormat.getDateTimeInstance(
        DateFormat.DEFAULT,
        DateFormat.DEFAULT,
        currentLocale
    )
}
