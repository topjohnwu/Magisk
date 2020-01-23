package com.topjohnwu.magisk.model.entity

import android.content.pm.ApplicationInfo
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.extensions.packageInfo
import com.topjohnwu.magisk.extensions.processes

class HideAppInfo(
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

class ProcessHideApp(
    val info: HideAppInfo,
    val processes: List<StatefulProcess>
)