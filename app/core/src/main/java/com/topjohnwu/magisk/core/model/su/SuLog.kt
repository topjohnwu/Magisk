package com.topjohnwu.magisk.core.model.su

import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.ktx.getLabel

@Entity(tableName = "logs")
class SuLog(
    val fromUid: Int,
    val toUid: Int,
    val fromPid: Int,
    val packageName: String,
    val appName: String,
    val command: String,
    val action: Int,
    val target: Int,
    val context: String,
    val gids: String,
    val time: Long = System.currentTimeMillis()
) {
    @PrimaryKey(autoGenerate = true) var id: Int = 0
}

fun PackageManager.createSuLog(
    info: ApplicationInfo,
    toUid: Int,
    fromPid: Int,
    command: String,
    policy: Int,
    target: Int,
    context: String,
    gids: String,
): SuLog {
    return SuLog(
        fromUid = info.uid,
        toUid = toUid,
        fromPid = fromPid,
        packageName = getNameForUid(info.uid)!!,
        appName = info.getLabel(this),
        command = command,
        action = policy,
        target = target,
        context = context,
        gids = gids,
    )
}

fun createSuLog(
    fromUid: Int,
    toUid: Int,
    fromPid: Int,
    command: String,
    policy: Int,
    target: Int,
    context: String,
    gids: String,
): SuLog {
    return SuLog(
        fromUid = fromUid,
        toUid = toUid,
        fromPid = fromPid,
        packageName = "[UID] $fromUid",
        appName = "[UID] $fromUid",
        command = command,
        action = policy,
        target = target,
        context = context,
        gids = gids,
    )
}
