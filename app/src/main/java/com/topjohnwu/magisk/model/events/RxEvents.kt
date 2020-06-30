package com.topjohnwu.magisk.model.events

import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.utils.RxBus
import org.json.JSONObject

sealed class PolicyUpdateEvent(val item: MagiskPolicy) : RxBus.Event {
    class Notification(item: MagiskPolicy) : PolicyUpdateEvent(item)
    class Log(item: MagiskPolicy) : PolicyUpdateEvent(item)
}

data class SafetyNetResult(
    val response: JSONObject? = null,
    val dismiss: Boolean = false
) : RxBus.Event
