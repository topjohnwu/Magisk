package com.topjohnwu.magisk.model.entity

import com.squareup.moshi.Json
import com.topjohnwu.magisk.utils.timeFormatStandard
import com.topjohnwu.magisk.utils.toTime
import timber.log.Timber

data class GithubRepo(
    @Json(name = "name") val name: String,
    @Json(name = "updated_at") val updatedAt: String
) {
    val updatedAtMillis by lazy {
        updatedAt.toTime(timeFormatStandard)
            .apply { Timber.i("$name updated @ $this ($updatedAt)") }
    }
}