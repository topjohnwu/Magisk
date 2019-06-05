package com.topjohnwu.magisk.model.entity

import se.ansman.kotshi.JsonSerializable

@JsonSerializable
data class MagiskApp(
    val version: String,
    val versionCode: String,
    val link: String,
    val note: String
)