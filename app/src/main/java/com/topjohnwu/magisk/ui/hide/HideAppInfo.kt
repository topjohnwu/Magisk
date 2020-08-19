package com.topjohnwu.magisk.ui.hide

import android.content.pm.ApplicationInfo
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.ktx.packageInfo
import com.topjohnwu.magisk.ktx.processes

data class HideAppInfo(
    val info: ApplicationInfo,
    val name: String,
    val icon: Drawable
) {
    val processes = info.packageInfo?.processes?.distinct() ?: listOf(info.packageName)
}

data class StatefulProcess(
    val name: String,
    val packageName: String,
    val isHidden: Boolean
)

data class HideAppTarget(
    val info: HideAppInfo,
    val processes: List<StatefulProcess>
)
