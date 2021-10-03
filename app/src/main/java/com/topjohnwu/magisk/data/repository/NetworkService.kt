package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CANARY_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.LITE_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.MagiskJson
import com.topjohnwu.magisk.core.model.StubJson
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.data.network.*
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class NetworkService(
    private val pages: GithubPageServices,
    private val raw: RawServices,
    private val jsd: JSDelivrServices,
    private val api: GithubApiServices
) {
    suspend fun fetchUpdate() = safe {
        var info = when (Config.updateChannel) {
            DEFAULT_CHANNEL, STABLE_CHANNEL -> fetchStableUpdate()
            BETA_CHANNEL -> fetchBetaUpdate()
            CANARY_CHANNEL -> fetchCanaryUpdate()
            LITE_CHANNEL -> fetchLiteUpdate()
            CUSTOM_CHANNEL -> fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.magisk.versionCode < Info.env.magiskVersionCode &&
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

    private suspend fun fetchLiteUpdate(): UpdateInfo {
        val sha = fetchLiteVersion()
        val info = jsd.fetchLiteUpdate(sha)

        fun genCDNUrl(name: String) = "${Const.Url.JS_DELIVR_URL}${MAGISK_LITE}@${sha}/${name}"
        fun MagiskJson.updateCopy() = copy(link = genCDNUrl(link))
        fun StubJson.updateCopy() = copy(link = genCDNUrl(link))

        return info.copy(
            magisk = info.magisk.updateCopy(),
            stub = info.stub.updateCopy()
        )
    }

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

    // Modules related
    suspend fun fetchRepoInfo(url: String = Const.Url.OFFICIAL_REPO) = safe {
        raw.fetchRepoInfo(url)
    }

    // Fetch files
    suspend fun fetchSafetynet() = wrap { jsd.fetchSafetynet() }
    suspend fun fetchBootctl() = wrap { jsd.fetchBootctl() }
    suspend fun fetchInstaller() = wrap {
        val sha = fetchMainVersion()
        jsd.fetchInstaller(sha)
    }
    suspend fun fetchFile(url: String) = wrap { raw.fetchFile(url) }
    suspend fun fetchString(url: String) = wrap { raw.fetchString(url) }

    private suspend fun fetchLiteVersion() = api.fetchBranch(MAGISK_LITE, "master").commit.sha
    private suspend fun fetchMainVersion() = api.fetchBranch(MAGISK_MAIN, "master").commit.sha
}
