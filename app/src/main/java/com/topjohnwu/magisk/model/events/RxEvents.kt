package com.topjohnwu.magisk.model.events

import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.utils.RxBus

sealed class PolicyUpdateEvent(val item: MagiskPolicy) : RxBus.Event {
    class Notification(item: MagiskPolicy) : PolicyUpdateEvent(item)
    class Log(item: MagiskPolicy) : PolicyUpdateEvent(item)
}

data class SafetyNetResult(val responseCode: Int) : RxBus.Event
