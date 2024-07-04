package com.topjohnwu.magisk.core.model.module

import android.os.Parcelable
import com.topjohnwu.magisk.core.model.ModuleJson
import kotlinx.parcelize.Parcelize

@Parcelize
data class OnlineModule(
    override var id: String,
    override var name: String,
    override var version: String,
    override var versionCode: Int,
    val zipUrl: String,
    val changelog: String,
) : Module(), Parcelable {
    constructor(local: LocalModule, json: ModuleJson) :
        this(local.id, local.name, json.version, json.versionCode, json.zipUrl, json.changelog)

    val downloadFilename get() = "$name-$version($versionCode).zip".legalFilename()

    private fun String.legalFilename() = replace(" ", "_")
        .replace("'", "").replace("\"", "")
        .replace("$", "").replace("`", "")
        .replace("*", "").replace("/", "_")
        .replace("#", "").replace("@", "")
        .replace("\\", "_")
}
