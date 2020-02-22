package com.topjohnwu.magisk.core.model

import android.os.Parcelable
import com.squareup.moshi.JsonClass
import kotlinx.android.parcel.Parcelize

@JsonClass(generateAdapter = true)
data class UpdateInfo(
    val app: ManagerJson = ManagerJson(),
    val uninstaller: UninstallerJson = UninstallerJson(),
    val magisk: MagiskJson = MagiskJson(),
    val stub: StubJson = StubJson()
)

@JsonClass(generateAdapter = true)
data class UninstallerJson(
    val link: String = ""
)

@JsonClass(generateAdapter = true)
data class MagiskJson(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = "",
    val md5: String = ""
)

@Parcelize
@JsonClass(generateAdapter = true)
data class ManagerJson(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = ""
) : Parcelable

@JsonClass(generateAdapter = true)
data class StubJson(
    val versionCode: Int = -1,
    val link: String = ""
)
