package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

data class MagiskConfig(
    val app: MagiskApp,
    val uninstaller: MagiskLink,
    val magisk: MagiskFlashable
)

@JsonClass(generateAdapter = true)
data class MagiskApp(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String
)

@JsonClass(generateAdapter = true)
data class MagiskLink(
    val link: String
)

@JsonClass(generateAdapter = true)
data class MagiskFlashable(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String,
    @Json(name = "md5") val hash: String
)