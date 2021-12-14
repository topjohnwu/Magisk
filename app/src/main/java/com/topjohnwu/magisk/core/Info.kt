package com.topjohnwu.magisk.core

import android.os.Build
import androidx.databinding.ObservableBoolean
import com.topjohnwu.magisk.DynAPK
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.core.utils.net.NetworkObserver
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.getProperty
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.ShellUtils.fastCmd
import com.topjohnwu.superuser.internal.UiThreadHandler

val isRunningAsStub get() = Info.stub != null

object Info {

    var stub: DynAPK.Data? = null

    val EMPTY_REMOTE = UpdateInfo()
    var remote = EMPTY_REMOTE
    suspend fun getRemote(svc: NetworkService): UpdateInfo? {
        return if (remote === EMPTY_REMOTE) {
            svc.fetchUpdate()?.apply { remote = this }
        } else remote
    }

    // Device state
    @JvmStatic val env by lazy { loadState() }
    @JvmStatic var isSAR = false
    var isAB = false
    val isVirtualAB = getProperty("ro.virtual_ab.enabled", "false") == "true"
    @JvmField val isZygiskEnabled = System.getenv("ZYGISK_ENABLED") == "1"
    @JvmStatic val isFDE get() = crypto == "block"
    @JvmField var ramdisk = false
    @JvmField var vbmeta = false
    @JvmField var hasGMS = true
    @JvmField val isPixel = Build.BRAND == "google"
    @JvmField val isEmulator =
        getProperty("ro.kernel.qemu", "0") == "1" ||
        getProperty("ro.boot.qemu", "0") == "1"
    var crypto = ""
    var noDataExec = false

    val isConnected by lazy {
        ObservableBoolean(false).also { field ->
            NetworkObserver.observe(AppContext) {
                UiThreadHandler.run { field.set(it) }
            }
        }
    }

    private fun loadState() = Env(
        fastCmd("magisk -v").split(":".toRegex())[0],
        runCatching { fastCmd("magisk -V").toInt() }.getOrDefault(-1)
    )

    class Env(
        val versionString: String = "",
        code: Int = -1
    ) {
        val versionCode = when {
            code < Const.Version.MIN_VERCODE -> -1
            else -> if (Shell.rootAccess()) code else -1
        }
        val isUnsupported = code > 0 && code < Const.Version.MIN_VERCODE
        val isActive = versionCode >= 0
    }
}
