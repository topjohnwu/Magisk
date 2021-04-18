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
import java.io.File
import java.io.IOException

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
    @JvmField var isSAR = false
    @JvmField var isAB = false
    @JvmField val isVirtualAB = getProperty("ro.virtual_ab.enabled", "false") == "true"
    @JvmStatic val isFDE get() = crypto == "block"
    @JvmField var ramdisk = false
    @JvmField var hasGMS = true
    @JvmField val isPixel = Build.BRAND == "google"
    @JvmField val isEmulator = getProperty("ro.kernel.qemu", "0") == "1"
    var crypto = ""
    var noDataExec = false

    val isConnected by lazy {
        ObservableBoolean(false).also { field ->
            NetworkObserver.observe(AppContext) {
                UiThreadHandler.run { field.set(it) }
            }
        }
    }

    val isNewReboot by lazy {
        try {
            val id = File("/proc/sys/kernel/random/boot_id").readText()
            if (id != Config.bootId) {
                Config.bootId = id
                true
            } else {
                false
            }
        } catch (e: IOException) {
            false
        }
    }

    private fun loadState() = Env(
        fastCmd("magisk -v").split(":".toRegex())[0],
        runCatching { fastCmd("magisk -V").toInt() }.getOrDefault(-1),
        Shell.su("magiskhide status").exec().isSuccess
    )

    class Env(
        val magiskVersionString: String = "",
        code: Int = -1,
        hide: Boolean = false
    ) {
        val magiskHide get() = Config.magiskHide
        val magiskVersionCode = when {
            code < Const.Version.MIN_VERCODE -> -1
            else -> if (Shell.rootAccess()) code else -1
        }
        val isUnsupported = code > 0 && code < Const.Version.MIN_VERCODE
        val isActive = magiskVersionCode >= 0

        init {
            Config.magiskHide = hide
        }
    }
}
