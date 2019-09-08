package com.topjohnwu.magisk.data.repository

import android.content.pm.PackageManager
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.data.database.base.su
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.extensions.getLabel
import com.topjohnwu.magisk.extensions.packageName
import com.topjohnwu.magisk.extensions.toSingle
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.superuser.Shell
import io.reactivex.Single

class MagiskRepository(
        private val apiRaw: GithubRawServices,
        private val packageManager: PackageManager
) {

    fun fetchSafetynet() = apiRaw.fetchSafetynet()

    fun fetchUpdate() = when (Config.updateChannel) {
        Config.Value.DEFAULT_CHANNEL, Config.Value.STABLE_CHANNEL -> apiRaw.fetchStableUpdate()
        Config.Value.BETA_CHANNEL -> apiRaw.fetchBetaUpdate()
        Config.Value.CANARY_CHANNEL -> apiRaw.fetchCanaryUpdate()
        Config.Value.CANARY_DEBUG_CHANNEL -> apiRaw.fetchCanaryDebugUpdate()
        Config.Value.CUSTOM_CHANNEL -> apiRaw.fetchCustomUpdate(Config.customChannelUrl)
        else -> throw IllegalArgumentException()
    }.flatMap {
        // If remote version is lower than current installed, try switching to beta
        if (it.magisk.versionCode < Info.magiskVersionCode
                && Config.updateChannel == Config.Value.DEFAULT_CHANNEL) {
            Config.updateChannel = Config.Value.BETA_CHANNEL
            apiRaw.fetchBetaUpdate()
        } else {
            Single.just(it)
        }
    }.doOnSuccess { Info.remote = it }

    fun fetchApps() =
        Single.fromCallable { packageManager.getInstalledApplications(0) }
            .flattenAsFlowable { it }
            .filter { it.enabled && !blacklist.contains(it.packageName) }
            .map {
                val label = it.getLabel(packageManager)
                val icon = it.loadIcon(packageManager)
                HideAppInfo(it, label, icon)
            }
            .filter { it.processes.isNotEmpty() }
            .toList()

    fun fetchHideTargets() = Shell.su("magiskhide --ls").toSingle()
        .map { it.exec().out }
        .flattenAsFlowable { it }
        .map { HideTarget(it) }
        .toList()

    fun toggleHide(isEnabled: Boolean, packageName: String, process: String) =
        "magiskhide --%s %s %s".format(isEnabled.state, packageName, process).su().ignoreElement()

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