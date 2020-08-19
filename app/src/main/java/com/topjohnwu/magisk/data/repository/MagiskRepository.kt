package com.topjohnwu.magisk.data.repository

import android.content.pm.PackageManager
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.ktx.await
import com.topjohnwu.magisk.ktx.getLabel
import com.topjohnwu.magisk.ktx.packageName
import com.topjohnwu.magisk.ui.hide.HideAppInfo
import com.topjohnwu.magisk.ui.hide.HideTarget
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.IOException

class MagiskRepository(
    private val apiRaw: GithubRawServices,
    private val packageManager: PackageManager
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
    }

    suspend fun fetchApps() = withContext(Dispatchers.Default) {
        packageManager.getInstalledApplications(0).filter {
            it.enabled && !blacklist.contains(it.packageName)
        }.map {
            val label = it.getLabel(packageManager)
            val icon = it.loadIcon(packageManager)
            HideAppInfo(it, label, icon)
        }.filter { it.processes.isNotEmpty() }
    }

    suspend fun fetchHideTargets() =
        Shell.su("magiskhide --ls").await().out.map { HideTarget(it) }

    fun toggleHide(isEnabled: Boolean, packageName: String, process: String) =
        Shell.su("magiskhide --${isEnabled.state} $packageName $process").submit()

    private val Boolean.state get() = if (this) "add" else "rm"

    companion object {
        private val blacklist by lazy { listOf(
            packageName,
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
        ) }
    }

}
