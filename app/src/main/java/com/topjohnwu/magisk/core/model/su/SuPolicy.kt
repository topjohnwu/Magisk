@file:SuppressLint("InlinedApi")
@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.core.model.su

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.core.model.su.SuPolicy.Companion.ALLOW
import com.topjohnwu.magisk.ktx.getLabel

data class SuPolicy(
    val uid: Int,
    val packageName: String,
    val appName: String,
    val icon: Drawable,
    var policy: Int = INTERACTIVE,
    val logging: Boolean = true,
    val notification: Boolean = true
) {

    companion object {
        const val INTERACTIVE = 0
        const val DENY = 1
        const val ALLOW = 2
    }

}

fun SuPolicy.toMap() = mapOf(
    "uid" to uid,
    "package_name" to packageName,
    "process" to packageName,
    "logging" to logging,
    "notification" to notification
)

@Throws(PackageManager.NameNotFoundException::class)
fun Map<String, String>.toPolicy(pm: PackageManager): SuPolicy? {
    val uid = get("uid")?.toIntOrNull()
    val packageName = get("package_name").orEmpty()
    val process = get("process")
    if (packageName.isEmpty() || process.isNullOrEmpty() || packageName != process) return null
    val info = pm.getApplicationInfo(packageName, 0)

    if (info.uid != uid)
        throw PackageManager.NameNotFoundException()

    return SuPolicy(
        uid = uid,
        packageName = packageName,
        appName = info.getLabel(pm),
        icon = info.loadIcon(pm),
        policy = ALLOW,
        logging = get("logging")?.toIntOrNull() != 0,
        notification = get("notification")?.toIntOrNull() != 0
    )
}

@Throws(PackageManager.NameNotFoundException::class)
fun Int.toPolicy(pm: PackageManager, policy: Int): SuPolicy {
    val pkg = pm.getPackagesForUid(this)?.firstOrNull()
        ?: throw PackageManager.NameNotFoundException()
    val info = pm.getApplicationInfo(pkg, 0)
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
