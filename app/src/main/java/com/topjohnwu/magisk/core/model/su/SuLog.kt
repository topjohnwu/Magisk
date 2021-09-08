package com.topjohnwu.magisk.core.model.su

import androidx.room.Entity
import androidx.room.PrimaryKey
import androidx.room.TypeConverter
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.ALLOW
import java.time.OffsetDateTime
import java.time.format.DateTimeFormatter

@Entity(tableName = "logs")
data class SuLog(
    val fromUid: Int,
    val toUid: Int,
    val fromPid: Int,
    val packageName: String,
    val appName: String,
    val command: String,
    val action: Boolean,
    val time: OffsetDateTime = OffsetDateTime.now()
) {
    @PrimaryKey(autoGenerate = true)
    var id: Int = 0
}

class Converters {
    private val formatter = DateTimeFormatter.ISO_OFFSET_DATE_TIME

    @TypeConverter
    fun toOffsetDateTime(value: String?) = value?.let {
        formatter.parse(it, OffsetDateTime::from)
    }

    @TypeConverter
    fun fromOffsetDateTime(date: OffsetDateTime?) = date?.format(formatter)
}

fun SuPolicy.toLog(
    toUid: Int,
    fromPid: Int,
    command: String
) = SuLog(uid, toUid, fromPid, packageName, appName, command, policy == ALLOW)
