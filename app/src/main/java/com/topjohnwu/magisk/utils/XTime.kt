package com.topjohnwu.magisk.utils

import java.text.DateFormat
import java.text.ParseException
import java.text.SimpleDateFormat

val now get() = System.currentTimeMillis()

fun Long.toTime(format: DateFormat) = format.format(this).orEmpty()
fun String.toTime(format: DateFormat) = try {
    format.parse(this)?.time ?: -1
} catch (e: ParseException) {
    -1L
}

private val locale get() = LocaleManager.locale
val timeFormatFull by lazy { SimpleDateFormat("yyyy/MM/dd_HH:mm:ss", locale) }
val timeFormatStandard by lazy { SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", locale) }
val timeFormatMedium by lazy { DateFormat.getDateInstance(DateFormat.MEDIUM, LocaleManager.locale) }
val timeFormatTime by lazy { SimpleDateFormat("h:mm a", LocaleManager.locale) }