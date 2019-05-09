package com.topjohnwu.magisk.data.repository

import android.content.Context
import android.content.pm.PackageManager
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.KConfig
import com.topjohnwu.magisk.data.database.base.su
import com.topjohnwu.magisk.data.database.base.suRaw
import com.topjohnwu.magisk.data.network.GithubRawApiServices
import com.topjohnwu.magisk.model.entity.HideAppInfo
import com.topjohnwu.magisk.model.entity.HideTarget
import com.topjohnwu.magisk.model.entity.Version
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.utils.toSingle
import com.topjohnwu.magisk.utils.writeToFile
import com.topjohnwu.superuser.Shell
import io.reactivex.Single
import io.reactivex.functions.BiFunction

class MagiskRepository(
    private val context: Context,
    private val apiRaw: GithubRawApiServices,
    private val packageManager: PackageManager
) {

    private val config = apiRaw.fetchConfig()
    private val configBeta = apiRaw.fetchBetaConfig()
    private val configCanary = apiRaw.fetchCanaryConfig()
    private val configCanaryDebug = apiRaw.fetchCanaryDebugConfig()


    fun fetchMagisk() = fetchConfig()
        .flatMap { apiRaw.fetchFile(it.magisk.link) }
        .map { it.writeToFile(context, FILE_MAGISK_ZIP) }

    fun fetchManager() = fetchConfig()
        .flatMap { apiRaw.fetchFile(it.app.link) }
        .map { it.writeToFile(context, FILE_MAGISK_APK) }

    fun fetchUninstaller() = fetchConfig()
        .flatMap { apiRaw.fetchFile(it.uninstaller.link) }
        .map { it.writeToFile(context, FILE_UNINSTALLER_ZIP) }

    fun fetchSafetynet() = apiRaw
        .fetchSafetynet()
        .map { it.writeToFile(context, FILE_SAFETY_NET_APK) }

    fun fetchBootctl() = apiRaw
        .fetchBootctl()
        .map { it.writeToFile(context, FILE_BOOTCTL_SH) }


    fun fetchConfig() = when (KConfig.updateChannel) {
        KConfig.UpdateChannel.STABLE -> config
        KConfig.UpdateChannel.BETA -> configBeta
        KConfig.UpdateChannel.CANARY -> configCanary
        KConfig.UpdateChannel.CANARY_DEBUG -> configCanaryDebug
    }


    fun fetchMagiskVersion(): Single<Version> = Single.zip(
        fetchMagiskVersionName(),
        fetchMagiskVersionCode(),
        BiFunction { versionName, versionCode ->
            Version(versionName, versionCode)
        }
    )


    fun fetchApps() =
        Single.fromCallable { packageManager.getInstalledApplications(0) }
            .flattenAsFlowable { it }
            .filter { it.enabled && !blacklist.contains(it.packageName) }
            .map {
                val label = Utils.getAppLabel(it, packageManager)
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

    private fun fetchMagiskVersionName() = "magisk -v".suRaw()
        .map { it.first() }
        .map { it.substring(0 until it.indexOf(":")) }
        .onErrorReturn { "Unknown" }

    private fun fetchMagiskVersionCode() = "magisk -V".suRaw()
        .map { it.first() }
        .map { it.toIntOrNull() ?: -1 }
        .onErrorReturn { -1 }

    fun toggleHide(isEnabled: Boolean, packageName: String, process: String) =
        "magiskhide --%s %s %s".format(isEnabled.state, packageName, process).su().ignoreElement()

    private val Boolean.state get() = if (this) "add" else "rm"

    companion object {
        const val FILE_MAGISK_ZIP = "magisk.zip"
        const val FILE_MAGISK_APK = "magisk.apk"
        const val FILE_UNINSTALLER_ZIP = "uninstaller.zip"
        const val FILE_SAFETY_NET_APK = "safetynet.apk"
        const val FILE_BOOTCTL_SH = "bootctl"

        private val blacklist = listOf(
            let { val app: App by inject(); app }.packageName,
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
        )
    }

}