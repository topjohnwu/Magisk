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
    } catch (e: Exception) {
        Timber.e(e)
        null
    }

    // UpdateInfo
    suspend fun fetchStableUpdate() = pages.fetchStableUpdate()
    suspend fun fetchBetaUpdate() = pages.fetchBetaUpdate()
    suspend fun fetchCustomUpdate(url: String) = raw.fetchCustomUpdate(url)
    suspend fun fetchCanaryUpdate(): UpdateInfo {
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

    suspend fun fetchRepoInfo(url: String = Const.Url.OFFICIAL_REPO) = try {
        raw.fetchRepoInfo(url)
    } catch (e: Exception) {
        Timber.e(e)
        null
    }

    suspend fun fetchSafetynet() = try {
        jsd.fetchSafetynet()
    } catch (e: HttpException) {
        throw IOException(e)
    }

    suspend fun fetchBootctl() = try {
        jsd.fetchBootctl()
    } catch (e: HttpException) {
        throw IOException(e)
    }

    suspend fun fetchInstaller() = try {
        val sha = fetchMainVersion()
        jsd.fetchInstaller(sha)
    } catch (e: HttpException) {
        throw IOException(e)
    }

    suspend fun fetchFile(url: String) = try {
        raw.fetchFile(url)
    } catch (e: HttpException) {
        throw IOException(e)
    }

    suspend fun fetchString(url: String) = try {
        raw.fetchString(url)
    } catch (e: Exception) {
        Timber.e(e)
        ""
    }

    private suspend fun fetchCanaryVersion() = api.fetchBranch(MAGISK_FILES, "canary").commit.sha
    private suspend fun fetchMainVersion() = api.fetchBranch(MAGISK_MAIN, "master").commit.sha
}
