package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.Json
import se.ansman.kotshi.JsonSerializable

@JsonSerializable
data class MagiskFlashable(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String,
    @Json(name = "md5") val hash: String
)