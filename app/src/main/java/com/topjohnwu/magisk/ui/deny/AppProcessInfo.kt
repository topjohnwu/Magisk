package com.topjohnwu.magisk.ui.deny

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
class AppProcessInfo(val applicationInfo: ApplicationInfo, pm: PackageManager,
                     denyList: List<CmdlineListItem>) : Comparable<AppProcessInfo> {

    private val denyList = denyList.filter {
        it.packageName == applicationInfo.packageName || it.packageName == ISOLATED_MAGIC
    }
    val label = applicationInfo.getLabel(pm)
    val iconImage: Drawable = applicationInfo.loadIcon(pm)
    val processes = fetchProcesses(pm)

    override fun compareTo(other: AppProcessInfo) = comparator.compare(this, other)

    private fun createProcess(name: String, pkg: String = applicationInfo.packageName) =
        ProcessInfo(name, pkg, denyList.any { it.process == name && it.packageName == pkg })

    private fun ComponentInfo.getProcName(): String = processName
        ?: applicationInfo.processName
        ?: applicationInfo.packageName

    private fun Array<out ComponentInfo>.processes() = map { createProcess(it.getProcName()) }

    private fun Array<ServiceInfo>.processes() = map {
        if ((it.flags and ServiceInfo.FLAG_ISOLATED_PROCESS) != 0) {
            if ((it.flags and ServiceInfo.FLAG_USE_APP_ZYGOTE) != 0) {
                val proc = applicationInfo.processName ?: applicationInfo.packageName
                createProcess("${proc}_zygote")
            } else {
                val proc = if (SDK_INT >= 29) "${it.getProcName()}:${it.name}" else it.getProcName()
                createProcess(proc, ISOLATED_MAGIC)
            }
        } else {
            createProcess(it.getProcName())
        }
    }

    private fun fetchProcesses(pm: PackageManager): List<ProcessInfo> {
        val flag = MATCH_DISABLED_COMPONENTS or MATCH_UNINSTALLED_PACKAGES or
            GET_ACTIVITIES or GET_SERVICES or GET_RECEIVERS or GET_PROVIDERS
        val packageInfo = try {
            pm.getPackageInfo(applicationInfo.packageName, flag)
        } catch (e: Exception) {
            // Exceed binder data transfer limit, local parsing package
            pm.getPackageArchiveInfo(applicationInfo.sourceDir, flag) ?: return emptyList()
        }

        val list = LinkedHashSet<ProcessInfo>()
        list += packageInfo.activities?.processes().orEmpty()
        list += packageInfo.services?.processes().orEmpty()
        list += packageInfo.receivers?.processes().orEmpty()
        list += packageInfo.providers?.processes().orEmpty()
        return list.sortedBy { it.name }
    }

    companion object {
        private val comparator = compareBy<AppProcessInfo>(
            { it.label.lowercase(currentLocale) },
            { it.applicationInfo.packageName }
        )
    }
}

data class ProcessInfo(
    val name: String,
    val packageName: String,
    var isEnabled: Boolean
) {
    val isIsolated get() = packageName == ISOLATED_MAGIC
    val isAppZygote get() = name.endsWith("_zygote")
}
