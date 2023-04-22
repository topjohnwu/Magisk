package com.topjohnwu.magisk.core.model

import android.os.Parcelable
import com.squareup.moshi.JsonClass
import kotlinx.parcelize.Parcelize

@JsonClass(generateAdapter = true)
data class UpdateInfo(
    val magisk: MagiskJson = MagiskJson(),
)

@Parcelize
@JsonClass(generateAdapter = true)
data class MagiskJson(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = ""
) : Parcelable

@JsonClass(generateAdapter = true)
data class ModuleJson(
    val version: String,
    val versionCode: Int,
    val zipUrl: String,
    val changelog: String,
)

@JsonClass(generateAdapter = true)
data class CommitInfo(
    val sha: String
)

@JsonClass(generateAdapter = true)
data class BranchInfo(
    val commit: CommitInfo
)
