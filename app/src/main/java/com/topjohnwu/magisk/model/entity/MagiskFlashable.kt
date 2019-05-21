package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class MagiskFlashable(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String,
    @Json(name = "md5") val hash: String
)