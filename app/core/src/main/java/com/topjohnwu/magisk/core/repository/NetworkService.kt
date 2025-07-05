package com.topjohnwu.magisk.core.repository

import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEBUG_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.data.GithubApiServices
import com.topjohnwu.magisk.core.data.RawUrl
import com.topjohnwu.magisk.core.ktx.dateFormat
import com.topjohnwu.magisk.core.model.Release
import com.topjohnwu.magisk.core.model.ReleaseAssets
import com.topjohnwu.magisk.core.model.UpdateInfo
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class NetworkService(
    private val raw: RawUrl,
    private val api: GithubApiServices,
) {
    suspend fun fetchUpdate() = safe {
        var info = when (Config.updateChannel) {
            DEFAULT_CHANNEL -> if (BuildConfig.DEBUG) fetchDebugUpdate() else fetchStableUpdate()
            STABLE_CHANNEL -> fetchStableUpdate()
            BETA_CHANNEL -> fetchBetaUpdate()
            DEBUG_CHANNEL -> fetchDebugUpdate()
            CUSTOM_CHANNEL -> fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.versionCode < Info.env.versionCode &&
            Config.updateChannel == DEFAULT_CHANNEL &&
            !BuildConfig.DEBUG
        ) {
            Config.updateChannel = BETA_CHANNEL
            info = fetchBetaUpdate()
        }
        info
    }

    suspend fun fetchUpdate(version: Int) = findRelease { it.versionCode == version }.asInfo()

    // Keep going through all release pages until we find a match
    private suspend inline fun findRelease(predicate: (Release) -> Boolean): Release? {
        var page = 1
        while (true) {
            val response = api.fetchReleases(page = page)
            val releases = response.body() ?: throw HttpException(response)
            // Remove all non Magisk releases
            releases.removeAll { it.tag[0] != 'v' && !it.tag.startsWith("canary") }
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

    private inline fun Release?.asInfo(
        selector: (ReleaseAssets) -> Boolean = {
            // Default selector picks the non-debug APK
            it.name.run { endsWith(".apk") && !contains("debug") }
        }): UpdateInfo {
        return if (this == null) UpdateInfo()
        else if (tag[0] == 'v') asPublicInfo(selector)
        else asCanaryInfo(selector)
    }

    private inline fun Release.asPublicInfo(selector: (ReleaseAssets) -> Boolean): UpdateInfo {
        val version = tag.drop(1)
        val date = dateFormat.format(createdTime)
        return UpdateInfo(
            version = version,
            versionCode = versionCode,
            link = assets.find(selector)!!.url,
            note = "## $date $name\n\n$body"
        )
    }

    private inline fun Release.asCanaryInfo(selector: (ReleaseAssets) -> Boolean): UpdateInfo {
        return UpdateInfo(
            version = name.substring(8, 16),
            versionCode = versionCode,
            link = assets.find(selector)!!.url,
            note = "## $name\n\n$body"
        )
    }

    // Version number: debug == beta >= stable

    // Find the latest non-prerelease
    private suspend fun fetchStableUpdate() = api.fetchLatestRelease().asInfo()

    // Find the latest release, regardless whether it's prerelease
    private suspend fun fetchBetaUpdate() = findRelease { true }.asInfo()

    private suspend fun fetchDebugUpdate() =
        findRelease { true }.asInfo { it.name == "app-debug.apk" }

    private suspend fun fetchCustomUpdate(url: String): UpdateInfo {
        val info = raw.fetchUpdateJson(url).magisk
        return info.let { it.copy(note = raw.fetchString(it.note)) }
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
