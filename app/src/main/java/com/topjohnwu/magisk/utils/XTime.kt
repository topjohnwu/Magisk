package com.topjohnwu.magisk.utils

import java.text.ParseException
import java.text.SimpleDateFormat
import java.util.*

val now get() = System.currentTimeMillis()

fun Long.toTime(format: SimpleDateFormat) = format.format(this).orEmpty()
fun String.toTime(format: SimpleDateFormat) = try {
    format.parse(this)?.time ?: -1
} catch (e: ParseException) {
    -1L
}

private val locale get() = Locale.getDefault()
val timeFormatFull by lazy { SimpleDateFormat("YYYY/MM/DD_HH:mm:ss", locale) }
val timeFormatStandard by lazy { SimpleDateFormat("YYYY-MM-DD'T'HH:mm:ss'Z'", locale) }