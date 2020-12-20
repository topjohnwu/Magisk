package com.topjohnwu.magisk.core

import android.os.Process

object Const {

    // Paths
    lateinit var MAGISKTMP: String
    val MAGISK_PATH get() = "$MAGISKTMP/modules"
    const val TMP_FOLDER_PATH = "/dev/tmp"
    const val MAGISK_LOG = "/cache/magisk.log"

    // Versions
    const val SNET_EXT_VER = 15
    const val SNET_REVISION = "18ab78817087c337ae0edd1ecac38aec49217880"
    const val BOOTCTL_REVISION = "18ab78817087c337ae0edd1ecac38aec49217880"

    // Misc
    val USER_ID = Process.myUid() / 100000

    object Version {
        const val MIN_VERSION = "v19.0"
        const val MIN_VERCODE = 19000

        fun atLeast_20_2() = Info.env.magiskVersionCode >= 20200 || isCanary()
        fun atLeast_20_4() = Info.env.magiskVersionCode >= 20400 || isCanary()
        fun atLeast_21_0() = Info.env.magiskVersionCode >= 21000 || isCanary()
        fun isCanary() = Info.env.magiskVersionCode % 100 != 0
    }

    object ID {
        const val FETCH_ZIP = 2
        const val SELECT_FILE = 3
        const val MAX_ACTIVITY_RESULT = 10

        // notifications
        const val MAGISK_UPDATE_NOTIFICATION_ID = 4
        const val APK_UPDATE_NOTIFICATION_ID = 5
        const val HIDE_MANAGER_NOTIFICATION_ID = 8
        const val UPDATE_NOTIFICATION_CHANNEL = "update"
        const val PROGRESS_NOTIFICATION_CHANNEL = "progress"
        const val CHECK_MAGISK_UPDATE_WORKER_ID = "magisk_update"
    }

    object Url {
        const val PATREON_URL = "https://www.patreon.com/topjohnwu"
        const val SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk"

        const val GITHUB_RAW_URL = "https://raw.githubusercontent.com/"
        const val GITHUB_API_URL = "https://api.github.com/"
        const val GITHUB_PAGE_URL = "https://topjohnwu.github.io/magisk_files/"
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
