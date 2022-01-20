@file:SuppressLint("InlinedApi")

package com.topjohnwu.magisk.core.model.su

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.INTERACTIVE
import com.topjohnwu.magisk.ktx.getLabel

data class SuPolicy(
    val uid: Int,
    val packageName: String,
    val appName: String,
    val icon: Drawable,
    var policy: Int = INTERACTIVE,
    var until: Long = -1L,
    val logging: Boolean = true,
    val notification: Boolean = true
) {

    companion object {
        const val INTERACTIVE = 0
        const val DENY = 1
        const val ALLOW = 2
    }

    fun toLog(toUid: Int, fromPid: Int, command: String) = SuLog(
        uid, toUid, fromPid, packageName, appName,
        command, policy == ALLOW)

    fun toMap() = mapOf(
        "uid" to uid,
        "package_name" to packageName,
        "policy" to policy,
        "until" to until,
        "logging" to logging,
        "notification" to notification
    )
}

@Throws(PackageManager.NameNotFoundException::class)
fun Map<String, String>.toPolicy(pm: PackageManager): SuPolicy {
    val uid = get("uid")?.toIntOrNull() ?: -1
    val packageName = get("package_name").orEmpty()
    val info = pm.getApplicationInfo(packageName, PackageManager.MATCH_UNINSTALLED_PACKAGES)

    if (info.uid != uid)
        throw PackageManager.NameNotFoundException()

    return SuPolicy(
        uid = uid,
        packageName = packageName,
        appName = info.getLabel(pm),
        icon = info.loadIcon(pm),
        policy = get("policy")?.toIntOrNull() ?: INTERACTIVE,
        until = get("until")?.toLongOrNull() ?: -1L,
        logging = get("logging")?.toIntOrNull() != 0,
        notification = get("notification")?.toIntOrNull() != 0
    )
}

@Throws(PackageManager.NameNotFoundException::class)
fun Int.toPolicy(pm: PackageManager, policy: Int = INTERACTIVE): SuPolicy {
    val pkg = pm.getPackagesForUid(this)?.firstOrNull()
        ?: throw PackageManager.NameNotFoundException()
    val info = pm.getApplicationInfo(pkg, PackageManager.MATCH_UNINSTALLED_PACKAGES)
    return SuPolicy(
        uid = info.uid,
        packageName = pkg,
        appName = info.getLabel(pm),
        icon = info.loadIcon(pm),
        policy = policy
    )
}

fun Int.toUidPolicy(pm: PackageManager, policy: Int): SuPolicy {
    return SuPolicy(
        uid = this,
        packageName = "[UID] $this",
        appName = "[UID] $this",
        icon = pm.defaultActivityIcon,
        policy = policy
    )
}
