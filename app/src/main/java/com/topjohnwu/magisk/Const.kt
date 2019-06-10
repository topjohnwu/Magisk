package com.topjohnwu.magisk

import android.os.Environment
import android.os.Process

import java.io.File

object Const {

    const val DEBUG_TAG = "MagiskManager"

    // Paths
    const val MAGISK_PATH = "/sbin/.magisk/img"
    @JvmField
    val EXTERNAL_PATH = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)!!
    @JvmField
    var MAGISK_DISABLE_FILE = File("xxx")
    const val TMP_FOLDER_PATH = "/dev/tmp"
    const val MAGISK_LOG = "/cache/magisk.log"

    // Versions
    const val SNET_EXT_VER = 12
    const val SNET_REVISION = "b66b1a914978e5f4c4bbfd74a59f4ad371bac107"
    const val BOOTCTL_REVISION = "9c5dfc1b8245c0b5b524901ef0ff0f8335757b77"

    // Misc
    const val ANDROID_MANIFEST = "AndroidManifest.xml"
    const val MAGISK_INSTALL_LOG_FILENAME = "magisk_install_log_%s.log"
    const val MANAGER_CONFIGS = ".tmp.magisk.config"
    @JvmField
    val USER_ID = Process.myUid() / 100000

    init {
        EXTERNAL_PATH.mkdirs()
    }

    object MagiskVersion {
        const val MIN_SUPPORT = 18000
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
        @Deprecated("This shouldn't be used. There's literally no need for it")
        const val REPO_URL =
            "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&sort=pushed&page=%d"
        const val FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s"
        const val ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip"
        const val MODULE_INSTALLER =
            "https://raw.githubusercontent.com/topjohnwu/Magisk/master/scripts/module_installer.sh"
        const val PAYPAL_URL = "https://www.paypal.me/topjohnwu"
        const val PATREON_URL = "https://www.patreon.com/topjohnwu"
        const val TWITTER_URL = "https://twitter.com/topjohnwu"
        const val XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382"
        const val SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk"
        @JvmField
        val BOOTCTL_URL = getRaw("9c5dfc1b8245c0b5b524901ef0ff0f8335757b77", "bootctl")
        const val GITHUB_RAW_API_URL = "https://raw.githubusercontent.com/"

        private fun getRaw(where: String, name: String) =
            "${GITHUB_RAW_API_URL}topjohnwu/magisk_files/$where/$name"
    }

    object Key {
        // others
        const val LINK_KEY = "Link"
        const val IF_NONE_MATCH = "If-None-Match"
        // intents
        const val OPEN_SECTION = "section"
        const val INTENT_SET_NAME = "filename"
        const val INTENT_SET_LINK = "link"
        const val FLASH_ACTION = "action"
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
