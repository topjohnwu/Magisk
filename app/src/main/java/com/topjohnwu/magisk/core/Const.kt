package com.topjohnwu.magisk.core

import android.os.Process
import java.io.File

object Const {

    // Paths
    lateinit var MAGISKTMP: String
    val MAGISK_PATH get() = "$MAGISKTMP/modules"
    var MAGISK_DISABLE_FILE = File("xxx")
    const val TMP_FOLDER_PATH = "/dev/tmp"
    const val MAGISK_LOG = "/cache/magisk.log"

    // Versions
    const val SNET_EXT_VER = 13
    const val SNET_REVISION = "a6c47f86f10b310358afa9dbe837037dd5d561df"
    const val BOOTCTL_REVISION = "a6c47f86f10b310358afa9dbe837037dd5d561df"

    // Misc
    const val ANDROID_MANIFEST = "AndroidManifest.xml"
    const val MAGISK_INSTALL_LOG_FILENAME = "magisk_install_log_%s.log"
    const val MANAGER_CONFIGS = ".tmp.magisk.config"
    val USER_ID = Process.myUid() / 100000

    object Version {
        const val MIN_VERSION = "v19.0"
        const val MIN_VERCODE = 19000
        const val PROVIDER_CONNECT = 20200
        const val DYNAMIC_PATH = 20400
    }

    object ID {
        const val FETCH_ZIP = 2
        const val SELECT_BOOT = 3

        // notifications
        const val MAGISK_UPDATE_NOTIFICATION_ID = 4
        const val APK_UPDATE_NOTIFICATION_ID = 5
        const val DTBO_NOTIFICATION_ID = 7
        const val HIDE_MANAGER_NOTIFICATION_ID = 8
        const val UPDATE_NOTIFICATION_CHANNEL = "update"
        const val PROGRESS_NOTIFICATION_CHANNEL = "progress"
        const val CHECK_MAGISK_UPDATE_WORKER_ID = "magisk_update"
    }

    object Url {
        const val ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip"
        const val PAYPAL_URL = "https://www.paypal.me/topjohnwu"
        const val PATREON_URL = "https://www.patreon.com/topjohnwu"
        const val TWITTER_URL = "https://twitter.com/topjohnwu"
        const val XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382"
        const val SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk"

        const val GITHUB_RAW_URL = "https://raw.githubusercontent.com/"
        const val GITHUB_API_URL = "https://api.github.com/users/Magisk-Modules-Repo/"
    }

    object Key {
        // others
        const val LINK_KEY = "Link"
        const val IF_NONE_MATCH = "If-None-Match"
        const val ETAG_KEY = "ETag"
        // intents
        const val OPEN_SECTION = "section"
        const val OPEN_SETTINGS = "settings"
        const val INTENT_SET_APP = "app_json"
        const val FLASH_INSTALLER = "installer"
        const val FLASH_ACTION = "action"
        const val FLASH_DATA = "additional_data"
        const val DISMISS_ID = "dismiss_id"
        const val BROADCAST_MANAGER_UPDATE = "manager_update"
        const val BROADCAST_REBOOT = "reboot"
    }

    object Value {
        const val FLASH_ZIP = "flash"
        const val PATCH_FILE = "patch"
        const val FLASH_MAGISK = "magisk"
        const val FLASH_INACTIVE_SLOT = "slot"
        const val UNINSTALL = "uninstall"
    }

}
