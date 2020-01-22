package com.topjohnwu.magisk.model.entity

import androidx.room.Entity
import androidx.room.Ignore
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.model.MagiskPolicy.Companion.ALLOW
import com.topjohnwu.magisk.extensions.now
import com.topjohnwu.magisk.extensions.timeFormatTime
import com.topjohnwu.magisk.extensions.toTime

@Entity(tableName = "logs")
data class MagiskLog(
    val fromUid: Int,
    val toUid: Int,
    val fromPid: Int,
    val packageName: String,
    val appName: String,
    val command: String,
    val action: Boolean,
    val time: Long = -1
) {
    @PrimaryKey(autoGenerate = true) var id: Int = 0
    @Ignore val timeString = time.toTime(timeFormatTime)
}

fun MagiskPolicy.toLog(
    toUid: Int,
    fromPid: Int,
    command: String
) = MagiskLog(uid, toUid, fromPid, packageName, appName, command, policy == ALLOW, now)
