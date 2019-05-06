package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class MagiskApp(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String
)