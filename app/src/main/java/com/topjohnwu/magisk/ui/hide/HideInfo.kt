package com.topjohnwu.magisk.ui.hide

import android.annotation.SuppressLint
import android.content.pm.ApplicationInfo
import android.content.pm.ComponentInfo
import android.content.pm.PackageManager
import android.content.pm.PackageManager.*
import android.content.pm.ServiceInfo
import android.graphics.drawable.Drawable
import android.os.Build.VERSION.SDK_INT
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.ktx.getLabel
import com.topjohnwu.magisk.ktx.isIsolated
import com.topjohnwu.magisk.ktx.useAppZygote

class CmdlineHiddenItem(line: String) {
    val packageName: String
    val process: String

    init {
        val split = line.split(Regex("\\|"), 2)
        packageName = split[0]
        process = split.getOrElse(1) { packageName }
    }
}

const val ISOLATED_MAGIC = "isolated"

@SuppressLint("InlinedApi")
class HideAppInfo(info: ApplicationInfo, pm: PackageManager, hideList: List<CmdlineHiddenItem>)
    : ApplicationInfo(info), Comparable<HideAppInfo> {

    val label = info.getLabel(pm)
    val iconImage: Drawable = info.loadIcon(pm)
    val processes = fetchProcesses(pm, hideList)

    override fun compareTo(other: HideAppInfo) = comparator.compare(this, other)

    private fun fetchProcesses(
        pm: PackageManager,
        hideList: List<CmdlineHiddenItem>
    ): List<HideProcessInfo> {
        // Fetch full PackageInfo
        val baseFlag = MATCH_DISABLED_COMPONENTS or MATCH_UNINSTALLED_PACKAGES
        val packageInfo = try {
            val request = GET_ACTIVITIES or GET_SERVICES or GET_RECEIVERS or GET_PROVIDERS
            pm.getPackageInfo(packageName, baseFlag or request)
        } catch (e: NameNotFoundException) {
            // EdXposed hooked, issue#3276
            return emptyList()
        } catch (e: Exception) {
            // Exceed binder data transfer limit, fetch each component type separately
            pm.getPackageInfo(packageName, baseFlag).apply {
                runCatching { activities = pm.getPackageInfo(packageName, baseFlag or GET_ACTIVITIES).activities }
                runCatching { services = pm.getPackageInfo(packageName, baseFlag or GET_SERVICES).services }
                runCatching { receivers = pm.getPackageInfo(packageName, baseFlag or GET_RECEIVERS).receivers }
                runCatching { providers = pm.getPackageInfo(packageName, baseFlag or GET_PROVIDERS).providers }
            }
        }

        val hidden = hideList.filter { it.packageName == packageName || it.packageName == ISOLATED_MAGIC }
        fun createProcess(name: String, pkg: String = packageName): HideProcessInfo {
            return HideProcessInfo(name, pkg, hidden.any { it.process == name && it.packageName == pkg })
        }

        var haveAppZygote = false
        fun Array<out ComponentInfo>.processes() = map { createProcess(it.processName) }
        fun Array<ServiceInfo>.processes() = map {
            if (it.isIsolated) {
                if (it.useAppZygote) {
                    haveAppZygote = true
                    // Using app zygote, don't need to track the process
                    null
                } else {
                    val proc = if (SDK_INT >= 29) "${it.processName}:${it.name}" else it.processName
                    createProcess(proc, ISOLATED_MAGIC)
                }
            } else {
                createProcess(it.processName)
            }
        }

        return with(packageInfo) {
            activities?.processes().orEmpty() +
            services?.processes().orEmpty() +
            receivers?.processes().orEmpty() +
            providers?.processes().orEmpty() +
            listOf(if (haveAppZygote) createProcess("${processName}_zygote") else null)
        }.filterNotNull().distinct().sortedBy { it.name }
    }

    companion object {
        private val comparator = compareBy<HideAppInfo>(
            { it.label.toLowerCase(currentLocale) },
            { it.packageName }
        )
    }
}

data class HideProcessInfo(
    val name: String,
    val packageName: String,
    var isHidden: Boolean
) {
    val isIsolated get() = packageName == ISOLATED_MAGIC
    val isAppZygote get() = name.endsWith("_zygote")
}
