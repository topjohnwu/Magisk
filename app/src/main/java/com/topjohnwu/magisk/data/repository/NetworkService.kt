package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Config.Value.BETA_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CANARY_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.CUSTOM_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.DEFAULT_CHANNEL
import com.topjohnwu.magisk.core.Config.Value.STABLE_CHANNEL
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.*
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
    }

    // UpdateInfo
    private suspend fun fetchStableUpdate() = pages.fetchStableUpdate()
    private suspend fun fetchBetaUpdate() = pages.fetchBetaUpdate()
    private suspend fun fetchCustomUpdate(url: String) = raw.fetchCustomUpdate(url)
    private suspend fun fetchCanaryUpdate(): UpdateInfo {
        val sha = fetchCanaryVersion()
        val info = jsd.fetchCanaryUpdate(sha)

        fun genCDNUrl(name: String) = "${Const.Url.JS_DELIVR_URL}${MAGISK_FILES}@${sha}/${name}"
        fun ManagerJson.updateCopy() = copy(link = genCDNUrl(link), note = genCDNUrl(note))
        fun MagiskJson.updateCopy() = copy(link = genCDNUrl(link), note = genCDNUrl(note))
        fun StubJson.updateCopy() = copy(link = genCDNUrl(link))
        fun UninstallerJson.updateCopy() = copy(link = genCDNUrl(link))

        return info.copy(
            app = info.app.updateCopy(),
            magisk = info.magisk.updateCopy(),
            stub = info.stub.updateCopy(),
            uninstaller = info.uninstaller.updateCopy()
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

    private suspend fun fetchCanaryVersion() = api.fetchBranch(MAGISK_FILES, "canary").commit.sha
    private suspend fun fetchMainVersion() = api.fetchBranch(MAGISK_MAIN, "master").commit.sha
}
