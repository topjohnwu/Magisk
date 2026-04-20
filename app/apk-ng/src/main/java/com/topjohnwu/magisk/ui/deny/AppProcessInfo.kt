package com.topjohnwu.magisk.ui.deny

import android.annotation.SuppressLint
import android.content.pm.ApplicationInfo
import android.content.pm.ComponentInfo
import android.content.pm.PackageManager
import android.content.pm.PackageManager.GET_ACTIVITIES
import android.content.pm.PackageManager.GET_PROVIDERS
import android.content.pm.PackageManager.GET_RECEIVERS
import android.content.pm.PackageManager.GET_SERVICES
import android.content.pm.PackageManager.MATCH_DISABLED_COMPONENTS
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.content.pm.ServiceInfo
import android.graphics.drawable.Drawable
import androidx.core.os.ProcessCompat
import com.topjohnwu.magisk.core.ktx.getLabel
import java.util.Locale
import java.util.TreeSet

class CmdlineListItem(line: String) {
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
class AppProcessInfo(
    private val info: ApplicationInfo,
    pm: PackageManager,
    denyList: List<CmdlineListItem>
) : Comparable<AppProcessInfo> {

    private val denyList = denyList.filter {
        it.packageName == info.packageName || it.packageName == ISOLATED_MAGIC
    }

    val label = info.getLabel(pm)
    val iconImage: Drawable = runCatching { info.loadIcon(pm) }.getOrDefault(pm.defaultActivityIcon)
    val packageName: String get() = info.packageName
    var firstInstallTime: Long = 0L
        private set
    var lastUpdateTime: Long = 0L
        private set
    val processes = fetchProcesses(pm)

    override fun compareTo(other: AppProcessInfo) = comparator.compare(this, other)

    fun isSystemApp() = info.flags and ApplicationInfo.FLAG_SYSTEM != 0

    fun isApp() = ProcessCompat.isApplicationUid(info.uid)

    private fun createProcess(name: String, pkg: String = info.packageName) =
        ProcessInfo(name, pkg, denyList.any { it.process == name && it.packageName == pkg })

    private fun ComponentInfo.getProcName(): String = processName
        ?: applicationInfo.processName
        ?: applicationInfo.packageName

    private val ServiceInfo.isIsolated get() = (flags and ServiceInfo.FLAG_ISOLATED_PROCESS) != 0
    private val ServiceInfo.useAppZygote get() = (flags and ServiceInfo.FLAG_USE_APP_ZYGOTE) != 0

    private fun Array<out ComponentInfo>?.toProcessList() =
        orEmpty().map { createProcess(it.getProcName()) }

    private fun Array<ServiceInfo>?.toProcessList(): List<ProcessInfo> {
        if (this == null) return emptyList()
        val result = mutableListOf<ProcessInfo>()
        var hasIsolated = false
        for (si in this) {
            if (si.isIsolated) {
                if (si.useAppZygote) {
                    val proc = info.processName ?: info.packageName
                    result.add(createProcess("${proc}_zygote"))
                } else {
                    hasIsolated = true
                }
            } else {
                result.add(createProcess(si.getProcName()))
            }
        }
        if (hasIsolated) {
            val prefix = "${info.processName ?: info.packageName}:"
            val isEnabled = denyList.any {
                it.packageName == ISOLATED_MAGIC && it.process.startsWith(prefix)
            }
            result.add(ProcessInfo(prefix, ISOLATED_MAGIC, isEnabled))
        }
        return result
    }

    private fun fetchProcesses(pm: PackageManager): Collection<ProcessInfo> {
        val flag = MATCH_DISABLED_COMPONENTS or MATCH_UNINSTALLED_PACKAGES or
            GET_ACTIVITIES or GET_SERVICES or GET_RECEIVERS or GET_PROVIDERS
        val packageInfo = try {
            pm.getPackageInfo(info.packageName, flag)
        } catch (e: Exception) {
            // Exceed binder data transfer limit, parse the package locally
            pm.getPackageArchiveInfo(info.sourceDir, flag) ?: return emptyList()
        }

        firstInstallTime = packageInfo.firstInstallTime
        lastUpdateTime = packageInfo.lastUpdateTime

        val processSet = TreeSet<ProcessInfo>(compareBy({ it.name }, { it.isIsolated }))
        processSet += packageInfo.activities.toProcessList()
        processSet += packageInfo.services.toProcessList()
        processSet += packageInfo.receivers.toProcessList()
        processSet += packageInfo.providers.toProcessList()
        return processSet
    }

    companion object {
        private val comparator = compareBy<AppProcessInfo>(
            { it.label.lowercase(Locale.ROOT) },
            { it.info.packageName }
        )
    }
}

data class ProcessInfo(
    val name: String,
    val packageName: String,
    var isEnabled: Boolean
) {
    val isIsolated = packageName == ISOLATED_MAGIC
}
