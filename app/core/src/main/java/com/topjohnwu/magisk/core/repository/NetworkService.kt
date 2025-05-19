package com.topjohnwu.magisk.core.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CANARY_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEBUG_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.data.GithubApiServices
import com.topjohnwu.magisk.core.data.RawUrl
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.Release
import com.topjohnwu.magisk.core.model.UpdateInfo
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException
import java.time.format.DateTimeFormatter

class NetworkService(
    private val raw: RawUrl,
    private val api: GithubApiServices,
) {
    suspend fun fetchUpdate() = safe {
        var info = when (Config.updateChannel) {
            DEFAULT_CHANNEL, STABLE_CHANNEL -> fetchStableUpdate()
            BETA_CHANNEL -> fetchBetaUpdate()
            CANARY_CHANNEL -> fetchCanaryUpdate()
            DEBUG_CHANNEL -> fetchDebugUpdate()
            CUSTOM_CHANNEL -> fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.magisk.versionCode < Info.env.versionCode &&
            Config.updateChannel == DEFAULT_CHANNEL) {
            Config.updateChannel = BETA_CHANNEL
            info = fetchBetaUpdate()
        }
        info
    }

    // Keep going through all release pages until we find a match
    private suspend inline fun findRelease(predicate: (Release) -> Boolean): Release? {
        var page = 1
        while (true) {
            val response = api.fetchReleases(page = page)
            val releases = response.body() ?: throw HttpException(response)
            // Make sure it's sorted correctly
            releases.sortByDescending { it.createdTime }
            releases.find(predicate)?.let { return it }
            if (response.headers()["link"]?.contains("rel=\"next\"", ignoreCase = true) == true) {
                page += 1
            } else {
                return null
            }
        }
    }

    private fun Release.asPublicInfo(): UpdateInfo {
        val version = tag.drop(1)
        val date = createdTime.format(DateTimeFormatter.ofPattern("yyyy.M.d"))
        val info = MagiskJson(
            version = version,
            versionCode = (version.toFloat() * 1000).toInt(),
            link = assets[0].url,
            note = "## $date $name\n\n$body"
        )
        return UpdateInfo(info)
    }

    private fun Release.asCanaryInfo(assetSelector: String): UpdateInfo {
        val info = MagiskJson(
            version = name.substring(8, 16),
            versionCode = tag.drop(7).toInt(),
            link = assets.find { it.name == assetSelector }!!.url,
            note = "## $name\n\n$body"
        )
        return UpdateInfo(info)
    }

    private suspend fun fetchStableUpdate() = api.fetchLatestRelease().asPublicInfo()

    private suspend fun fetchBetaUpdate() = findRelease { it.tag[0] == 'v' }!!.asPublicInfo()

    private suspend fun fetchCanary() = findRelease { it.tag.startsWith("canary-") }!!

    private suspend fun fetchCanaryUpdate() = fetchCanary().asCanaryInfo("app-release.apk")

    private suspend fun fetchDebugUpdate() = fetchCanary().asCanaryInfo("app-debug.apk")

    private suspend fun fetchCustomUpdate(url: String): UpdateInfo {
        val info = raw.fetchUpdateJson(url).magisk
        return UpdateInfo(info.let { it.copy(note = raw.fetchString(it.note)) })
    }

    private inline fun <T> safe(factory: () -> T): T? {
        return try {
            if (Info.isConnected.value == true)
                factory()
            else
                null
        } catch (e: Exception) {
            Timber.e(e)
            null
        }
    }

    private inline fun <T> wrap(factory: () -> T): T {
        return try {
            factory()
        } catch (e: HttpException) {
            throw IOException(e)
        }
    }

    // Fetch files
    suspend fun fetchFile(url: String) = wrap { raw.fetchFile(url) }
    suspend fun fetchString(url: String) = wrap { raw.fetchString(url) }
    suspend fun fetchModuleJson(url: String) = wrap { raw.fetchModuleJson(url) }
}
