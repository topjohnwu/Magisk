package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CANARY_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.data.network.GithubApiServices
import com.topjohnwu.magisk.data.network.GithubPageServices
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.data.network.JSDelivrServices
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class NetworkService(
    private val pages: GithubPageServices,
    private val raw: GithubRawServices,
    private val jsd: JSDelivrServices,
    private val api: GithubApiServices
) {
    suspend fun fetchUpdate() = try {
        var info = when (Config.updateChannel) {
            DEFAULT_CHANNEL, STABLE_CHANNEL -> fetchStableUpdate()
            BETA_CHANNEL -> fetchBetaUpdate()
            CANARY_CHANNEL -> fetchCanaryUpdate()
            CUSTOM_CHANNEL -> fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.magisk.versionCode < Info.env.magiskVersionCode &&
            Config.updateChannel == DEFAULT_CHANNEL
        ) {
            Config.updateChannel = BETA_CHANNEL
            info = fetchBetaUpdate()
        }
        Info.remote = info
        info
    } catch (e: IOException) {
        Timber.e(e)
        null
    } catch (e: HttpException) {
        Timber.e(e)
        null
    }

    // UpdateInfo
    suspend fun fetchStableUpdate() = pages.fetchStableUpdate()
    suspend fun fetchBetaUpdate() = pages.fetchBetaUpdate()
    suspend fun fetchCanaryUpdate() = raw.fetchCanaryUpdate()
    suspend fun fetchCustomUpdate(url: String) = raw.fetchCustomUpdate(url)

    // Byte streams
    suspend fun fetchSafetynet() = jsd.fetchSafetynet()
    suspend fun fetchBootctl() = jsd.fetchBootctl()
    suspend fun fetchInstaller() = raw.fetchInstaller()
    suspend fun fetchFile(url: String) = raw.fetchFile(url)

    // Strings
    suspend fun fetchMetadata(repo: Repo) = raw.fetchModuleFile(repo.id, "module.prop")
    suspend fun fetchReadme(repo: Repo) = raw.fetchModuleFile(repo.id, "README.md")
    suspend fun fetchString(url: String) = raw.fetchString(url)

    // API calls
    suspend fun fetchRepos(page: Int, etag: String) = api.fetchRepos(page, etag)
}
