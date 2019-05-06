package com.topjohnwu.magisk.model.entity

data class MagiskConfig(
    val app: MagiskApp,
    val uninstaller: MagiskLink,
    val magisk: MagiskFlashable
)
