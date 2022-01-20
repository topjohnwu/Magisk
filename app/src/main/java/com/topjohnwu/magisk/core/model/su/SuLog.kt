package com.topjohnwu.magisk.core.model.su

import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.ktx.now

@Entity(tableName = "logs")
data class SuLog(
    val fromUid: Int,
    val toUid: Int,
    val fromPid: Int,
    val packageName: String,
    val appName: String,
    val command: String,
    val action: Boolean,
    val time: Long = now
) {
    @PrimaryKey(autoGenerate = true) var id: Int = 0
}
