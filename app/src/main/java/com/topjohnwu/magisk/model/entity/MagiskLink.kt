package com.topjohnwu.magisk.model.entity

import se.ansman.kotshi.JsonSerializable

@JsonSerializable
data class MagiskLink(
    val link: String
)