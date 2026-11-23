package com.topjohnwu.magisk.wear.home

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info

/**
 * What the home screen shows about the installed Magisk. [version] and [versionCode] are only
 * set when a supported Magisk is active, the same rule the phone app uses for its version text.
 */
data class HomeStatus(
    val version: String?,
    val versionCode: Int?,
    val installed: Boolean,
    val zygisk: Boolean,
    val ramdisk: Boolean,
) {
    companion object
}

/** The status of [env], with [Info.isRooted] deciding whether its version can be trusted. */
fun HomeStatus.Companion.from(env: Info.Env, zygisk: Boolean, ramdisk: Boolean): HomeStatus {
    // Env keeps the raw version code private: its versionCode is already -1 when the daemon
    // is unreachable, unsupported or the app has no root, which of() treats the same way
    return of(env.versionString, env.versionCode, Info.isRooted, zygisk, ramdisk)
}

/**
 * Build the status from the raw values that [Info.Env] is created from, following the rule
 * of [Info.Env.versionCode] and [Info.Env.isActive]: Magisk counts as installed only when
 * [code] is at least [Const.Version.MIN_VERCODE] and the app is [rooted].
 */
fun HomeStatus.Companion.of(
    versionString: String,
    code: Int,
    rooted: Boolean,
    zygisk: Boolean,
    ramdisk: Boolean,
): HomeStatus {
    val installed = code >= Const.Version.MIN_VERCODE && rooted
    return HomeStatus(
        version = versionString.takeIf { installed && it.isNotEmpty() },
        versionCode = code.takeIf { installed },
        installed = installed,
        zygisk = zygisk,
        ramdisk = ramdisk,
    )
}
