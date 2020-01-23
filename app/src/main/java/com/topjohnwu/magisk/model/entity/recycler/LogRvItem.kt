package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.timeDateFormat
import com.topjohnwu.magisk.extensions.toTime
import com.topjohnwu.magisk.model.entity.MagiskLog

class LogItem(val item: MagiskLog) : ObservableItem<LogItem>() {

    override val layoutRes = R.layout.item_log_access_md2

    val date = item.time.toTime(timeDateFormat)
    var isTop = false
        @Bindable get
        set(value) {
            field = value
            notifyChange(BR.top)
        }
    var isBottom = false
        @Bindable get
        set(value) {
            field = value
            notifyChange(BR.bottom)
        }

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
