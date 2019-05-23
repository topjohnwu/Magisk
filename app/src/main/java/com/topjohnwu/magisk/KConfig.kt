package com.topjohnwu.magisk

import com.chibatching.kotpref.ContextProvider
import com.chibatching.kotpref.KotprefModel
import com.topjohnwu.magisk.KConfig.UpdateChannel.STABLE
import com.topjohnwu.magisk.utils.get

object KConfig : KotprefModel(get<ContextProvider>()) {
    override val kotprefName: String = "${context.packageName}_preferences"

    private var internalUpdateChannel by intPref(STABLE.id, "updateChannel")
    var useCustomTabs by booleanPref(true, "useCustomTabs")
    @JvmStatic
    var customUpdateChannel by stringPref("", "custom_channel")

    @JvmStatic
    var updateChannel: UpdateChannel
        get() = UpdateChannel.byId(internalUpdateChannel)
        set(value) {
            internalUpdateChannel = value.id
        }

    internal const val DEFAULT_CHANNEL = "topjohnwu/magisk_files"

    enum class UpdateChannel(val id: Int) {

        STABLE(Config.Value.STABLE_CHANNEL),
        BETA(Config.Value.BETA_CHANNEL),
        CANARY(Config.Value.CANARY_CHANNEL),
        CANARY_DEBUG(Config.Value.CANARY_DEBUG_CHANNEL),
        CUSTOM(Config.Value.CUSTOM_CHANNEL);

        companion object {
            fun byId(id: Int) = when (id) {
                Config.Value.STABLE_CHANNEL -> STABLE
                Config.Value.BETA_CHANNEL -> BETA
                Config.Value.CUSTOM_CHANNEL -> CUSTOM
                Config.Value.CANARY_CHANNEL -> CANARY
                Config.Value.CANARY_DEBUG_CHANNEL -> CANARY_DEBUG
                else -> STABLE
            }
        }
    }
}