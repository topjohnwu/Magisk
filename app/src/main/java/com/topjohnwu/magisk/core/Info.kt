package com.topjohnwu.magisk.core

import android.os.Build
import androidx.databinding.ObservableBoolean
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.StubApk
import com.topjohnwu.magisk.core.model.UpdateInfo
import com.topjohnwu.magisk.core.utils.net.NetworkObserver
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.ktx.getProperty
import com.topjohnwu.superuser.ShellUtils.fastCmd
import com.topjohnwu.superuser.internal.UiThreadHandler
import java.util.*

val isRunningAsStub get() = Info.stub != null

object Info {

    var stub: StubApk.Data? = null

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
    var isAB = false
    @JvmField val isZygiskEnabled = System.getenv("ZYGISK_ENABLED") == "1"
    @JvmStatic val isFDE get() = crypto == "block"
    @JvmField var ramdisk = false
    @JvmField var vbmeta = false
    var crypto = ""
    var noDataExec = false
    var isRooted = false

    @JvmField var hasGMS = true
    val isSamsung = Build.MANUFACTURER.equals("samsung", ignoreCase = true)
    @JvmField val isEmulator =
        getProperty("ro.kernel.qemu", "0") == "1" ||
            getProperty("ro.boot.qemu", "0") == "1"

    val isConnected by lazy {
        ObservableBoolean(false).also { field ->
            NetworkObserver.observe(AppContext) {
                UiThreadHandler.run { field.set(it) }
            }
        }
    }

    val constInfo by lazy {
        HashMap<String, String>().apply {
            put("root", isRooted.toString())
            put("stub", isRunningAsStub.toString())
            put("runningVer", env.versionString)
            put("runningVerCode", env.versionCode.toString())
            put("appVer", BuildConfig.VERSION_NAME)
            put("appVerCode", BuildConfig.VERSION_CODE.toString())
            put("zygisk", isZygiskEnabled.toString())
            put("isSAR", isSAR.toString())
            put("isAB", isAB.toString())
            put("crypto", crypto)
            put("ramdisk", ramdisk.toString())
            put("noDataExec", noDataExec.toString())
            put("isEmulator", isEmulator.toString())
            put("supportedABIs", Arrays.toString(Build.SUPPORTED_ABIS))
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
            else -> if (isRooted) code else -1
        }
        val isUnsupported = code > 0 && code < Const.Version.MIN_VERCODE
        val isActive = versionCode >= 0
    }
}
