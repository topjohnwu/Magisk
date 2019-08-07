package com.topjohnwu.magisk.model.entity

import com.topjohnwu.magisk.extensions.timeFormatTime
import com.topjohnwu.magisk.extensions.toTime
import com.topjohnwu.magisk.model.entity.MagiskPolicy.Companion.ALLOW
import java.util.*

data class MagiskLog(
    val fromUid: Int,
    val toUid: Int,
    val fromPid: Int,
    val packageName: String,
    val appName: String,
    val command: String,
    val action: Boolean,
    val date: Date
) {
    val timeString = date.time.toTime(timeFormatTime)
}

data class WrappedMagiskLog(
    val time: Long,
    val items: List<MagiskLog>
)

fun Map<String, String>.toLog(): MagiskLog {
    return MagiskLog(
        fromUid = get("from_uid")?.toIntOrNull() ?: -1,
        toUid = get("to_uid")?.toIntOrNull() ?: -1,
        fromPid = get("from_pid")?.toIntOrNull() ?: -1,
        packageName = get("package_name").orEmpty(),
        appName = get("app_name").orEmpty(),
        command = get("command").orEmpty(),
        action = get("action")?.toIntOrNull() != 0,
        date = get("time")?.toLongOrNull()?.toDate() ?: Date()
    )
}

fun Long.toDate() = Date(this)

fun MagiskLog.toMap() = mapOf(
    "from_uid" to fromUid,
    "to_uid" to toUid,
    "from_pid" to fromPid,
    "package_name" to packageName,
    "app_name" to appName,
    "command" to command,
    "action" to action,
    "time" to date.time
)

fun MagiskPolicy.toLog(
    toUid: Int,
    fromPid: Int,
    command: String,
    date: Date
) = MagiskLog(uid, toUid, fromPid, packageName, appName, command, policy == ALLOW, date)
