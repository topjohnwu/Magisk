package com.topjohnwu.magisk.model.events

import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem
import com.topjohnwu.magisk.model.entity.recycler.ModuleRvItem
import com.topjohnwu.magisk.model.entity.recycler.PolicyRvItem

class HideProcessEvent(val item: HideProcessRvItem) : RxBus.Event

class PolicyEnableEvent(val item: PolicyRvItem, val enable: Boolean) : RxBus.Event
sealed class PolicyUpdateEvent(val item: MagiskPolicy) : RxBus.Event {
    class Notification(item: MagiskPolicy) : PolicyUpdateEvent(item)
    class Log(item: MagiskPolicy) : PolicyUpdateEvent(item)
}

class ModuleUpdatedEvent(val item: ModuleRvItem) : RxBus.Event
