package com.topjohnwu.magisk.core

import android.os.Build
import android.os.Process
import com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE

@Suppress("DEPRECATION")
object Const {

    val CPU_ABI: String get() = Build.SUPPORTED_ABIS[0]

    // Null if 32-bit only or 64-bit only
    val CPU_ABI_32 =
        if (Build.SUPPORTED_64_BIT_ABIS.isEmpty()) null
        else Build.SUPPORTED_32_BIT_ABIS.firstOrNull()

    // Paths
    const val MODULE_PATH  = "/data/adb/modules"
    const val TMPDIR = "/dev/tmp"
    const val MAGISK_LOG = "/cache/magisk.log"

    // Misc
    val USER_ID = Process.myUid() / 100000

    object Version {
        const val MIN_VERSION = "v22.0"
        const val MIN_VERCODE = 22000

        private fun isCanary() = (Info.env.versionCode % 100) != 0
        fun atLeast_24_0() = Info.env.versionCode >= 24000 || isCanary()
        fun atLeast_25_0() = Info.env.versionCode >= 25000 || isCanary()
        fun atLeast_28_0() = Info.env.versionCode >= 28000 || isCanary()
        fun atLeast_30_1() = Info.env.versionCode >= 30100 || isCanary()
    }

    object ID {
        const val DOWNLOAD_JOB_ID = 6
        const val CHECK_UPDATE_JOB_ID = 7
    }

    object Url {
        const val PATREON_URL = "https://www.patreon.com/topjohnwu"
        const val SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk"

        const val GITHUB_API_URL = "https://api.github.com/"
        const val GITHUB_PAGE_URL = "https://topjohnwu.github.io/magisk-files/"
        const val INVALID_URL = "https://example.com/"
    }

    object Key {
        // intents
        const val OPEN_SECTION = "section"
        const val PREV_CONFIG = "prev_config"
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
        const val MODULES = "modules"
        const val SUPERUSER = "superuser"
    }
}
