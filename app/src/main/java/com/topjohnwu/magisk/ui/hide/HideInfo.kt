package com.topjohnwu.magisk.ui.hide

import android.annotation.SuppressLint
import android.content.pm.ApplicationInfo
import android.content.pm.ComponentInfo
import android.content.pm.PackageManager
import android.content.pm.PackageManager.*
import android.content.pm.ServiceInfo
import android.graphics.drawable.Drawable
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.ktx.getLabel
import com.topjohnwu.magisk.ktx.isIsolated

class CmdlineHiddenItem(line: String) {
    val packageName: String
    val process: String
    val uid: Int

    init {
        val split = line.split(Regex("\\|"), 3)
        uid = split[0].toInt()
        packageName = split[1]
        process = split.getOrElse(2) { packageName }
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

        val hidden = hideList.filter { it.packageName == packageName && it.uid == uid }
        fun createProcess(name: String): HideProcessInfo {
            return HideProcessInfo(name, packageName, uid, hidden.any { it.process == name })
        }

        fun Array<out ComponentInfo>.processes() = map { createProcess(it.processName) }
        fun Array<ServiceInfo>.processes() = map {
            if (it.isIsolated) {
                null
            } else {
                createProcess(it.processName)
            }
        }

        return with(packageInfo) {
            activities?.processes().orEmpty() +
            services?.processes().orEmpty() +
            receivers?.processes().orEmpty() +
            providers?.processes().orEmpty()
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
    val uid: Int,
    var isHidden: Boolean
) {
    val isIsolated get() = packageName == ISOLATED_MAGIC
    val isAppZygote get() = name.endsWith("_zygote")
}
