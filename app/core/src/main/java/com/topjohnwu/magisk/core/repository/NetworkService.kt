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
import com.topjohnwu.magisk.core.data.RawServices
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.Release
import com.topjohnwu.magisk.core.model.UpdateInfo
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class NetworkService(
    private val raw: RawServices,
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

    // UpdateInfo
    private suspend fun fetchStableUpdate(rel: Release? = null): UpdateInfo {
        val release = rel ?: api.fetchLatestRelease()
        val name = release.tag_name.drop(1)
        val info = MagiskJson(
            name, (name.toFloat() * 1000).toInt(),
            release.assets[0].browser_download_url, release.body
        )
        return UpdateInfo(info)
    }

    private suspend fun fetchBetaUpdate(): UpdateInfo {
        val release = api.fetchRelease().find { it.tag_name[0] == 'v' && it.prerelease }
        return fetchStableUpdate(release)
    }

    private suspend fun fetchCanaryUpdate(): UpdateInfo {
        val release = api.fetchRelease().find { it.tag_name.startsWith("canary-") }
        val info = MagiskJson(
            release!!.name.substring(8, 16),
            release.tag_name.drop(7).toInt(),
            release.assets.find { it.name == "app-release.apk" }!!.browser_download_url,
            release.body
        )
        return UpdateInfo(info)
    }

    private suspend fun fetchDebugUpdate(): UpdateInfo {
        val release = fetchCanaryUpdate()
        val link = release.magisk.link.replace("app-release.apk", "app-debug.apk")
        return UpdateInfo(release.magisk.copy(link = link))
    }

    private suspend fun fetchCustomUpdate(url: String) = raw.fetchUpdateJSON(url)

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
