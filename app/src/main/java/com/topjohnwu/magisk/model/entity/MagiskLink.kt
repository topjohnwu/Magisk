package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class MagiskLink(
    val link: String
)