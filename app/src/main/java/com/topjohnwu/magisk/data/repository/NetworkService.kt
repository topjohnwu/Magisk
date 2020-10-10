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
import okhttp3.ResponseBody
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

    // Modules related
    suspend fun fetchRepoInfo(url: String = Const.Url.OFFICIAL_REPO) = raw.fetchRepoInfo(url)

    // Fetch files
    suspend fun fetchSafetynet() = jsd.fetchSafetynet()
    suspend fun fetchBootctl() = jsd.fetchBootctl()
    suspend fun fetchInstaller(): ResponseBody {
        val sha = fetchMainVersion()
        return jsd.fetchInstaller(sha)
    }
    suspend fun fetchFile(url: String) = raw.fetchFile(url)
    suspend fun fetchString(url: String) = raw.fetchString(url)

    private suspend fun fetchCanaryVersion() = api.fetchBranch(MAGISK_FILES, "canary").commit.sha
    private suspend fun fetchMainVersion() = api.fetchBranch(MAGISK_MAIN, "master").commit.sha
}
