package com.topjohnwu.magisk.model.events

import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem
import com.topjohnwu.magisk.model.entity.recycler.PolicyRvItem

class HideProcessEvent(val item: HideProcessRvItem) : RxBus.Event

class PolicyEnableEvent(val item: PolicyRvItem, val enable: Boolean) : RxBus.Event
sealed class PolicyUpdateEvent(val item: PolicyRvItem) : RxBus.Event {
    class Notification(item: PolicyRvItem) : PolicyUpdateEvent(item)
    class Log(item: PolicyRvItem) : PolicyUpdateEvent(item)
}
