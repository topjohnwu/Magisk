package com.topjohnwu.magisk.model.entity

import android.os.Parcelable
import com.squareup.moshi.Json
import kotlinx.android.parcel.Parcelize
import se.ansman.kotshi.JsonSerializable

@JsonSerializable
data class UpdateInfo(
    val app: ManagerJson = ManagerJson(),
    val uninstaller: UninstallerJson = UninstallerJson(),
    val magisk: MagiskJson = MagiskJson()
)

@JsonSerializable
data class UninstallerJson(
    val link: String = ""
)

@Parcelize
@JsonSerializable
data class MagiskJson(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = "",
    @Json(name = "md5") val hash: String = ""
) : Parcelable

@JsonSerializable
data class ManagerJson(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = ""
)
