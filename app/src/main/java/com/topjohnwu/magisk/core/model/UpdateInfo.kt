package com.topjohnwu.magisk.core.model

import android.os.Parcelable
import com.squareup.moshi.JsonClass
import kotlinx.parcelize.Parcelize

@JsonClass(generateAdapter = true)
data class UpdateInfo(
    val magisk: MagiskJson = MagiskJson(),
    val stub: StubJson = StubJson()
)

@Parcelize
@JsonClass(generateAdapter = true)
data class MagiskJson(
    val version: String = "N/A",
    val versionCode: Int = -1,
    val link: String = "x",
    val note: String = "x"
) : Parcelable

@Parcelize
@JsonClass(generateAdapter = true)
data class StubJson(
    val versionCode: Int = -1,
    val link: String = "x"
) : Parcelable

@JsonClass(generateAdapter = true)
data class ModuleJson(
    val id: String,
    val last_update: Long,
    val prop_url: String,
    val zip_url: String,
    val notes_url: String
)

@JsonClass(generateAdapter = true)
data class CommitInfo(
    val sha: String
)

@JsonClass(generateAdapter = true)
data class BranchInfo(
    val commit: CommitInfo
)
