package com.topjohnwu.magisk.model.events

import com.skoumal.teanity.rxbus.RxBus
import com.topjohnwu.magisk.model.entity.recycler.HideProcessRvItem

class HideProcessEvent(val item: HideProcessRvItem) : RxBus.Event