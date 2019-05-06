package com.topjohnwu.magisk

import androidx.appcompat.app.AppCompatDelegate
import com.chibatching.kotpref.KotprefModel
import com.topjohnwu.magisk.KConfig.UpdateChannel.*

object KConfig : KotprefModel() {
    override val kotprefName: String = "${context.packageName}_preferences"

    var darkMode by intPref(default = AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM, key = "darkMode")
    var magiskChecksum by stringPref("", "magiskChecksum")
    var forceEncrypt by booleanPref(false, "forceEncryption")
    var keepVerity by booleanPref(false, "keepVerity")
    var bootFormat by stringPref("img", "bootFormat")
    var suLogTimeout by longPref(0, "suLogTimeout")
    private var internalUpdateChannel by stringPref(
        KConfig.UpdateChannel.STABLE.toString(),
        "updateChannel"
    )
    var useCustomTabs by booleanPref(true, "useCustomTabs")

    var updateChannel: UpdateChannel
        get() = valueOf(internalUpdateChannel)
        set(value) {
            internalUpdateChannel = value.toString()
        }

    val isStable get() = !(isCanary || isBeta)
    val isCanary get() = updateChannel == CANARY || updateChannel == CANARY_DEBUG
    val isBeta get() = updateChannel == BETA


    enum class UpdateChannel {
        STABLE, BETA, CANARY, CANARY_DEBUG
    }
}