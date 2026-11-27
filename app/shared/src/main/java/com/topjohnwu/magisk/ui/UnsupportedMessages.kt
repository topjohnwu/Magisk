package com.topjohnwu.magisk.ui

import android.content.Context
import android.content.pm.ApplicationInfo
import com.topjohnwu.magisk.core.Info
import java.io.File
import com.topjohnwu.magisk.core.R as CoreR

/**
 * Problems with the Magisk installation or the app's own install that the user should be
 * told about on startup, as pairs of title and message string resources.
 */
fun Context.unsupportedMessages(): List<Pair<Int, Int>> {
    val messages = mutableListOf<Pair<Int, Int>>()

    if (Info.env.isUnsupported) {
        messages.add(CoreR.string.unsupport_magisk_title to CoreR.string.unsupport_magisk_msg)
    }
    if (!Info.isEmulator && Info.env.isActive && System.getenv("PATH")
            ?.split(':')
            ?.filterNot { File("$it/magisk").exists() }
            ?.any { File("$it/su").exists() } == true) {
        messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_other_su_msg)
    }
    if (applicationInfo.flags and ApplicationInfo.FLAG_SYSTEM != 0) {
        messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_system_app_msg)
    }
    if (applicationInfo.flags and ApplicationInfo.FLAG_EXTERNAL_STORAGE != 0) {
        messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_external_storage_msg)
    }

    return messages
}
