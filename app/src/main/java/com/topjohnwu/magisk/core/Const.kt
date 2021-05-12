package com.topjohnwu.magisk.core

import android.os.Build
import android.os.Process
import com.topjohnwu.magisk.BuildConfig
import java.io.File

@Suppress("DEPRECATION")
object Const {

    val CPU_ABI: String = Build.SUPPORTED_ABIS[0]
    val CPU_ABI_32: String = Build.SUPPORTED_32_BIT_ABIS.firstOrNull() ?: CPU_ABI

    // Paths
    lateinit var MAGISKTMP: String
    lateinit var NATIVE_LIB_DIR: File
    val MAGISK_PATH get() = "$MAGISKTMP/modules"
    const val TMPDIR = "/dev/tmp"
    const val MAGISK_LOG = "/cache/magisk.log"

    // Versions
    const val SNET_EXT_VER = 17
    const val SNET_REVISION = "23.0"
    const val BOOTCTL_REVISION = "22.0"

    // Misc
    val USER_ID = Process.myUid() / 100000

    object Version {
        const val MIN_VERSION = "v20.4"
        const val MIN_VERCODE = 20400

        fun atLeast_21_0() = Info.env.magiskVersionCode >= 21000 || isCanary()
        fun atLeast_21_2() = Info.env.magiskVersionCode >= 21200 || isCanary()
        fun isCanary() = Info.env.magiskVersionCode % 100 != 0
    }

    object ID {
        // notifications
        const val APK_UPDATE_NOTIFICATION_ID = 5
        const val UPDATE_NOTIFICATION_CHANNEL = "update"
        const val PROGRESS_NOTIFICATION_CHANNEL = "progress"
        const val CHECK_MAGISK_UPDATE_WORKER_ID = "magisk_update"
    }

    object Url {
        const val PATREON_URL = "https://www.patreon.com/topjohnwu"
        const val SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk"

        val CHANGELOG_URL = if (BuildConfig.VERSION_CODE % 100 != 0) Info.remote.magisk.note
        else "https://topjohnwu.github.io/Magisk/releases/${BuildConfig.VERSION_CODE}.md"

        const val GITHUB_RAW_URL = "https://raw.githubusercontent.com/"
        const val GITHUB_API_URL = "https://api.github.com/"
        const val GITHUB_PAGE_URL = "https://topjohnwu.github.io/magisk-files/"
        const val JS_DELIVR_URL = "https://cdn.jsdelivr.net/gh/"
        const val OFFICIAL_REPO = "https://magisk-modules-repo.github.io/submission/modules.json"
    }

    object Key {
        // intents
        const val OPEN_SECTION = "section"
        const val PREV_PKG = "prev_pkg"
    }

    object Value {
        const val FLASH_ZIP = "flash"
        const val PATCH_FILE = "patch"
        const val FLASH_MAGISK = "magisk"
        const val FLASH_INACTIVE_SLOT = "slot"
        const val UNINSTALL = "uninstall"
    }

    object Nav {
        const val HOME = "home"
        const val SETTINGS = "settings"
        const val HIDE = "hide"
        const val MODULES = "modules"
        const val SUPERUSER = "superuser"
    }
}
