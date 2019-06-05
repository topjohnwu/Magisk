package com.topjohnwu.magisk

import android.content.Context
import com.topjohnwu.magisk.KConfig.UpdateChannel.STABLE
import com.topjohnwu.magisk.di.Protected
import com.topjohnwu.magisk.model.preference.PreferenceModel
import com.topjohnwu.magisk.utils.inject

object KConfig : PreferenceModel() {

    override val context: Context by inject(Protected)
    override val fileName: String = "${context.packageName}_preferences"

    private var internalUpdateChannel by preference(Config.Key.UPDATE_CHANNEL, STABLE.id.toString())
    var useCustomTabs by preference("useCustomTabs", true)
    @JvmStatic
    var customUpdateChannel by preference(Config.Key.CUSTOM_CHANNEL, "")

    @JvmStatic
    var updateChannel: UpdateChannel
        get() = UpdateChannel.byId(internalUpdateChannel.toIntOrNull() ?: STABLE.id)
        set(value) {
            internalUpdateChannel = value.id.toString()
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