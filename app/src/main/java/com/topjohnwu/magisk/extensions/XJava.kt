package com.topjohnwu.magisk.extensions

import android.net.Uri
import android.os.Build
import androidx.core.net.toFile
import java.io.File
import java.io.InputStream
import java.io.OutputStream
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import kotlin.NoSuchElementException

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

fun String.langTagToLocale(): Locale {
    if (Build.VERSION.SDK_INT >= 21) {
        return Locale.forLanguageTag(this)
    } else {
        val tok = split("-".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
        if (tok.isEmpty()) {
            return Locale("")
        }
        val language = when (tok[0]) {
            "und" -> "" // Undefined
            "fil" -> "tl" // Filipino
            else -> tok[0]
        }
        if (language.length != 2 && language.length != 3)
            return Locale("")
        if (tok.size == 1)
            return Locale(language)
        val country = tok[1]

        return if (country.length != 2 && country.length != 3) Locale(language)
        else Locale(language, country)
    }
}

fun Locale.toLangTag(): String {
    if (Build.VERSION.SDK_INT >= 21) {
        return toLanguageTag()
    } else {
        var language = language
        var country = country
        var variant = variant
        when {
            language.isEmpty() || !language.matches("\\p{Alpha}{2,8}".toRegex()) ->
                language = "und"       // Follow the Locale#toLanguageTag() implementation
            language == "iw" -> language = "he"        // correct deprecated "Hebrew"
            language == "in" -> language = "id"        // correct deprecated "Indonesian"
            language == "ji" -> language = "yi"        // correct deprecated "Yiddish"
        }
        // ensure valid country code, if not well formed, it's omitted

        // variant subtags that begin with a letter must be at least 5 characters long
        // ensure valid country code, if not well formed, it's omitted
        if (!country.matches("\\p{Alpha}{2}|\\p{Digit}{3}".toRegex())) {
            country = ""
        }

        // variant subtags that begin with a letter must be at least 5 characters long
        if (!variant.matches("\\p{Alnum}{5,8}|\\p{Digit}\\p{Alnum}{3}".toRegex())) {
            variant = ""
        }
        val tag = StringBuilder(language)
        if (country.isNotEmpty())
            tag.append('-').append(country)
        if (variant.isNotEmpty())
            tag.append('-').append(variant)
        return tag.toString()
    }
}