package com.topjohnwu.magisk.ktx

import com.topjohnwu.magisk.core.utils.currentLocale
import java.text.DateFormat
import java.text.SimpleDateFormat

val now get() = System.currentTimeMillis()

fun Long.toTime(format: DateFormat) = format.format(this).orEmpty()

val timeFormatStandard by lazy { SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss",
    currentLocale
) }

val timeDateFormat: DateFormat by lazy {
    DateFormat.getDateTimeInstance(
        DateFormat.DEFAULT,
        DateFormat.DEFAULT,
        currentLocale
    )
}
