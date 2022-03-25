package com.topjohnwu.magisk.core.model.su

import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.ktx.getLabel
import com.topjohnwu.magisk.ktx.getPackageInfo

class SuPolicy(
    val uid: Int,
    val packageName: String,
    val appName: String,
    val icon: Drawable,
    var policy: Int = INTERACTIVE,
    var until: Long = -1L,
    var logging: Boolean = true,
    var notification: Boolean = true
) {

    companion object {
        const val INTERACTIVE = 0
        const val DENY = 1
        const val ALLOW = 2
    }

    fun toMap() = mapOf(
        "uid" to uid,
        "package_name" to packageName,
        "policy" to policy,
        "until" to until,
        "logging" to logging,
        "notification" to notification
    )
}

fun PackageManager.createPolicy(info: PackageInfo): SuPolicy {
    val appInfo = info.applicationInfo
    val prefix = if (info.sharedUserId == null) "" else "[SharedUID] "
    return SuPolicy(
        uid = appInfo.uid,
        packageName = getNameForUid(appInfo.uid)!!,
        appName = "$prefix${appInfo.getLabel(this)}",
        icon = appInfo.loadIcon(this),
    )
}

@Throws(PackageManager.NameNotFoundException::class)
fun PackageManager.createPolicy(uid: Int): SuPolicy {
    val info = getPackageInfo(uid, -1)
    return if (info == null) {
        // We can assert getNameForUid does not return null because
        // getPackageInfo will already throw if UID does not exist
        val name = getNameForUid(uid)!!
        SuPolicy(
            uid = uid,
            packageName = name,
            appName = "[SharedUID] $name",
            icon = defaultActivityIcon,
        )
    } else {
        createPolicy(info)
    }
}

@Throws(PackageManager.NameNotFoundException::class)
fun PackageManager.createPolicy(map: Map<String, String>): SuPolicy {
    val uid = map["uid"]?.toIntOrNull() ?: throw IllegalArgumentException()
    val policy = createPolicy(uid)

    map["policy"]?.toInt()?.let { policy.policy = it }
    map["until"]?.toLong()?.let { policy.until = it }
    map["logging"]?.toInt()?.let { policy.logging = it != 0 }
    map["notification"]?.toInt()?.let { policy.notification = it != 0 }
    return policy
}
