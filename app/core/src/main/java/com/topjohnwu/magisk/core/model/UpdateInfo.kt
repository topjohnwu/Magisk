package com.topjohnwu.magisk.core.model

import android.os.Parcelable
import com.squareup.moshi.FromJson
import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass
import com.squareup.moshi.JsonQualifier
import com.squareup.moshi.ToJson
import kotlinx.parcelize.Parcelize
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter.ISO_OFFSET_DATE_TIME

@JsonClass(generateAdapter = true)
class UpdateJson(
    val magisk: UpdateInfo = UpdateInfo(),
)

@Parcelize
@JsonClass(generateAdapter = true)
data class UpdateInfo(
    val version: String = "",
    val versionCode: Int = -1,
    val link: String = "",
    val note: String = ""
) : Parcelable

@JsonClass(generateAdapter = true)
data class ModuleJson(
    val version: String,
    val versionCode: Int,
    val zipUrl: String,
    val changelog: String,
)

@JsonClass(generateAdapter = true)
data class ReleaseAssets(
    val name: String,
    @Json(name = "browser_download_url") val url: String,
)

class DateTimeAdapter {
    @ToJson
    fun toJson(date: LocalDateTime): String {
        return date.toString()
    }

    @FromJson
    fun fromJson(date: String): LocalDateTime {
        return LocalDateTime.parse(date, ISO_OFFSET_DATE_TIME)
    }
}

@JsonClass(generateAdapter = true)
data class Release(
    @Json(name = "tag_name") val tag: String,
    val name: String,
    val prerelease: Boolean,
    val assets: List<ReleaseAssets>,
    val body: String,
    @Json(name = "created_at") val createdTime: LocalDateTime,
) {
    val versionCode: Int get() {
        return if (tag[0] == 'v') {
            (tag.drop(1).toFloat() * 1000).toInt()
        } else {
            tag.drop(7).toInt()
        }
    }
}
