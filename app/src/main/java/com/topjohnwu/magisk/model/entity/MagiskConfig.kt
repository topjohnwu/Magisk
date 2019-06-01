package com.topjohnwu.magisk.model.entity

import se.ansman.kotshi.JsonSerializable

@JsonSerializable
data class MagiskConfig(
    val app: MagiskApp,
    val uninstaller: MagiskLink,
    val magisk: MagiskFlashable
)
