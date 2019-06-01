package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class MagiskConfig(
    val app: MagiskApp,
    val uninstaller: MagiskLink,
    val magisk: MagiskFlashable
)
