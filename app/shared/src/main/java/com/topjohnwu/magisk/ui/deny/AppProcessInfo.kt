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
import android.os.Process
import androidx.compose.runtime.Immutable
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
const val WEBVIEW_ZYGOTE_MAGIC = "webview_zygote"
const val WEBVIEW_ZYGOTE_UID = 1053

@Immutable
data class ProcessInfo(
    val name: String,
    val packageName: String,
    val isEnabled: Boolean,
) {
    val isIsolated: Boolean = packageName == ISOLATED_MAGIC
}

@Immutable
@SuppressLint("InlinedApi")
data class AppProcessInfo(
    val packageName: String,
    val label: String,
    val isSystemApp: Boolean,
    val isApp: Boolean,
    val firstInstallTime: Long,
    val lastUpdateTime: Long,
    val processes: List<ProcessInfo>,
) : Comparable<AppProcessInfo> {

    override fun compareTo(other: AppProcessInfo): Int = comparator.compare(this, other)

    companion object {
        operator fun invoke(
            info: ApplicationInfo,
            pm: PackageManager,
            denyList: List<CmdlineListItem>,
        ): AppProcessInfo {
            val appDenyList = denyList.filter {
                it.packageName == info.packageName || it.packageName == ISOLATED_MAGIC
            }

            fun createProcess(name: String, pkg: String = info.packageName) =
                ProcessInfo(name, pkg, appDenyList.any { it.process == name && it.packageName == pkg })

            fun ComponentInfo.getProcName(): String = processName
                ?: applicationInfo.processName
                ?: applicationInfo.packageName

            fun Array<out ComponentInfo>?.toProcessList() =
                orEmpty().map { createProcess(it.getProcName()) }

            fun Array<ServiceInfo>?.toServiceProcessList(): List<ProcessInfo> {
                if (this == null) return emptyList()
                val result = mutableListOf<ProcessInfo>()
                var hasIsolated = false
                for (si in this) {
                    val isIsolated = (si.flags and ServiceInfo.FLAG_ISOLATED_PROCESS) != 0
                    val useAppZygote = (si.flags and ServiceInfo.FLAG_USE_APP_ZYGOTE) != 0
                    if (isIsolated) {
                        if (useAppZygote) {
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
                    val isEnabled = appDenyList.any {
                        it.packageName == ISOLATED_MAGIC && it.process.startsWith(prefix)
                    }
                    result.add(ProcessInfo(prefix, ISOLATED_MAGIC, isEnabled))
                }
                return result
            }

            val flag = MATCH_DISABLED_COMPONENTS or MATCH_UNINSTALLED_PACKAGES or
                GET_ACTIVITIES or GET_SERVICES or GET_RECEIVERS or GET_PROVIDERS
            val packageInfo = try {
                pm.getPackageInfo(info.packageName, flag)
            } catch (e: Exception) {
                // Exceed binder data transfer limit, parse the package locally
                pm.getPackageArchiveInfo(info.sourceDir, flag)
            }

            val firstInstallTime = packageInfo?.firstInstallTime ?: 0L
            val lastUpdateTime = packageInfo?.lastUpdateTime ?: 0L

            val processSet = TreeSet<ProcessInfo>(compareBy({ it.name }, { it.isIsolated }))
            if (packageInfo != null) {
                processSet += packageInfo.activities.toProcessList()
                processSet += packageInfo.services.toServiceProcessList()
                processSet += packageInfo.receivers.toProcessList()
                processSet += packageInfo.providers.toProcessList()
            }

            return AppProcessInfo(
                packageName = info.packageName,
                label = info.getLabel(pm),
                isSystemApp = info.flags and ApplicationInfo.FLAG_SYSTEM != 0,
                isApp = Process.isApplicationUid(info.uid),
                firstInstallTime = firstInstallTime,
                lastUpdateTime = lastUpdateTime,
                processes = processSet.toList(),
            )
        }

        fun webViewZygote(
            pm: PackageManager,
            denyList: List<CmdlineListItem>,
            label: String,
        ): AppProcessInfo {
            val enabled = denyList.any {
                it.packageName == WEBVIEW_ZYGOTE_MAGIC &&
                    it.process == WEBVIEW_ZYGOTE_MAGIC
            }
            return AppProcessInfo(
                packageName = WEBVIEW_ZYGOTE_MAGIC,
                label = label,
                isSystemApp = false,
                isApp = true,
                firstInstallTime = 0L,
                lastUpdateTime = 0L,
                processes = listOf(
                    ProcessInfo(WEBVIEW_ZYGOTE_MAGIC, WEBVIEW_ZYGOTE_MAGIC, enabled)
                ),
            )
        }

        private val comparator = compareBy<AppProcessInfo>(
            { it.label.lowercase(Locale.ROOT) },
            { it.packageName }
        )
    }
}
