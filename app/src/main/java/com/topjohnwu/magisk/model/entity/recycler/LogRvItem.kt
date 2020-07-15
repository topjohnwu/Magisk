package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ktx.timeDateFormat
import com.topjohnwu.magisk.ktx.toTime
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.magisk.utils.set

class LogItem(val item: MagiskLog) : ObservableItem<LogItem>() {

    override val layoutRes = R.layout.item_log_access_md2

    val date = item.time.toTime(timeDateFormat)

    @get:Bindable
    var isTop = false
        set(value) = set(value, field, { field = it }, BR.top)

    @get:Bindable
    var isBottom = false
        set(value) = set(value, field, { field = it }, BR.bottom)

    override fun itemSameAs(other: LogItem) = item.appName == other.item.appName

    override fun contentSameAs(other: LogItem) = item.fromUid == other.item.fromUid &&
            item.toUid == other.item.toUid &&
            item.fromPid == other.item.fromPid &&
            item.packageName == other.item.packageName &&
            item.command == other.item.command &&
            item.action == other.item.action &&
            item.time == other.item.time &&
            isTop == other.isTop &&
            isBottom == other.isBottom
}
