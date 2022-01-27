package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CANARY_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubPageServices
import com.topjohnwu.magisk.data.network.RawServices
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class NetworkService(
    private val pages: GithubPageServices,
    private val raw: RawServices,
    private val api: GithubApiServices
) {
    suspend fun fetchUpdate() = safe {
        var info = when (Config.updateChannel) {
            DEFAULT_CHANNEL, STABLE_CHANNEL -> fetchStableUpdate()
            BETA_CHANNEL -> fetchBetaUpdate()
            CANARY_CHANNEL -> fetchCanaryUpdate()
            CUSTOM_CHANNEL -> fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.magisk.versionCode < Info.env.versionCode &&
            Config.updateChannel == DEFAULT_CHANNEL
        ) {
            Config.updateChannel = BETA_CHANNEL
            info = fetchBetaUpdate()
        }
        info
    }

    // UpdateInfo
    private suspend fun fetchStableUpdate() = pages.fetchUpdateJSON("stable.json")
    private suspend fun fetchBetaUpdate() = pages.fetchUpdateJSON("beta.json")
    private suspend fun fetchCanaryUpdate() = pages.fetchUpdateJSON("canary.json")
    private suspend fun fetchCustomUpdate(url: String) = raw.fetchCustomUpdate(url)

    private inline fun <T> safe(factory: () -> T): T? {
        return try {
            factory()
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
