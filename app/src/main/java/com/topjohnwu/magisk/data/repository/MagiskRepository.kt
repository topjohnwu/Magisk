package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.network.GithubRawServices
import retrofit2.HttpException
import timber.log.Timber
import java.io.IOException

class MagiskRepository(
    private val apiRaw: GithubRawServices
) {

    suspend fun fetchUpdate() = try {
        var info = when (Config.updateChannel) {
            Config.Value.DEFAULT_CHANNEL, Config.Value.STABLE_CHANNEL -> apiRaw.fetchStableUpdate()
            Config.Value.BETA_CHANNEL -> apiRaw.fetchBetaUpdate()
            Config.Value.CANARY_CHANNEL -> apiRaw.fetchCanaryUpdate()
            Config.Value.CUSTOM_CHANNEL -> apiRaw.fetchCustomUpdate(Config.customChannelUrl)
            else -> throw IllegalArgumentException()
        }
        if (info.magisk.versionCode < Info.env.magiskVersionCode &&
            Config.updateChannel == Config.Value.DEFAULT_CHANNEL) {
            Config.updateChannel = Config.Value.BETA_CHANNEL
            info = apiRaw.fetchBetaUpdate()
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

}
