package com.topjohnwu.magisk.extensions

import android.content.res.Resources

val specialChars = arrayOf('!', '@', '#', '$', '%', '&', '?')

fun String.replaceRandomWithSpecial(): String {
    var random: Char
    do {
        random = random()
    } while (random == '.')
    return replace(random, specialChars.random())
}

fun StringBuilder.appendIf(condition: Boolean, builder: StringBuilder.() -> Unit) =
    if (condition) apply(builder) else this

fun Int.res(vararg args: Any): String {
    val resources: Resources by inject()
    return resources.getString(this, *args)
}

fun String.trimEmptyToNull(): String? = if (isBlank()) null else this

fun String.legalFilename() = replace(" ", "_").replace("'", "").replace("\"", "")
        .replace("$", "").replace("`", "").replace("*", "").replace("/", "_")
        .replace("#", "").replace("@", "").replace("\\", "_")

fun String.isEmptyInternal() = isNullOrBlank()