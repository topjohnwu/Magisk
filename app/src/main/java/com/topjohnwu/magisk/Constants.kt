package com.topjohnwu.magisk

import android.os.Process

object Constants {

    // Paths
    val MAGISK_PATH = "/sbin/.magisk/img"
    val MAGISK_LOG = "/cache/magisk.log"

    val USER_ID get() = Process.myUid() / 100000

    const val SNET_REVISION = "b66b1a914978e5f4c4bbfd74a59f4ad371bac107"
    const val BOOTCTL_REVISION = "9c5dfc1b8245c0b5b524901ef0ff0f8335757b77"

    const val GITHUB_URL = "https://github.com/"
    const val GITHUB_API_URL = "https://api.github.com/"
    const val GITHUB_RAW_API_URL = "https://raw.githubusercontent.com/"

}