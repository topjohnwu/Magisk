package com.topjohnwu.magisk.ui

import android.content.pm.PackageManager
import android.os.Build

internal const val POST_NOTIFICATIONS_PERMISSION = "android.permission.POST_NOTIFICATIONS"

internal val MATCH_UNINSTALLED_PACKAGES_COMPAT: Int
    @Suppress("DEPRECATION")
    get() = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
        PackageManager.MATCH_UNINSTALLED_PACKAGES
    } else {
        PackageManager.GET_UNINSTALLED_PACKAGES
    }
