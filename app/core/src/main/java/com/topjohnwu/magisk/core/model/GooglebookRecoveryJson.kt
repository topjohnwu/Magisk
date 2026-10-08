package com.topjohnwu.magisk.core.model

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class GooglebookRecoveryJson(
    val channel: String = "",
    val device: String = "",
    val file: String = "",
    val filesize: Long = 0L,
    val manufacturer: String = "",
    val md5: String = "",
    val model: String = "",
    val name: String = "",
    val url: String = "",
    val zipfilesize: Long = 0L,
    val buildexploreruri: String = "",
    val buildidentifier: String = "",
    val target: String = "",
    val branch: String = "",
    val dateupdated: String = "",
    val previousVersionGeneration: Long = 0L,
)
