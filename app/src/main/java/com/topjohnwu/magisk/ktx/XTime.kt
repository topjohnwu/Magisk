package com.topjohnwu.magisk.ktx

import com.topjohnwu.magisk.core.utils.currentLocale
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

val timeFormatFull by lazy { SimpleDateFormat("yyyy/MM/dd_HH:mm:ss",
    currentLocale
) }
val timeFormatStandard by lazy { SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'",
    currentLocale
) }
val timeFormatMedium by lazy { DateFormat.getDateInstance(DateFormat.MEDIUM,
    currentLocale
) }
val timeFormatTime by lazy { SimpleDateFormat("h:mm a",
    currentLocale
) }
val timeDateFormat by lazy {
    DateFormat.getDateTimeInstance(
        DateFormat.DEFAULT,
        DateFormat.DEFAULT,
        currentLocale
    )
}
