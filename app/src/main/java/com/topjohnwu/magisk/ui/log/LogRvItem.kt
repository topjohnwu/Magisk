package com.topjohnwu.magisk.ui.log

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ktx.timeDateFormat
import com.topjohnwu.magisk.ktx.toTime
import com.topjohnwu.magisk.utils.set

class LogRvItem(val item: SuLog) : ObservableItem<LogRvItem>() {

    override val layoutRes = R.layout.item_log_access_md2

    val date = item.time.toTime(timeDateFormat)

    @get:Bindable
    var isTop = false
        set(value) = set(value, field, { field = it }, BR.top)

    @get:Bindable
    var isBottom = false
        set(value) = set(value, field, { field = it }, BR.bottom)

    override fun itemSameAs(other: LogRvItem) = item.appName == other.item.appName

    override fun contentSameAs(other: LogRvItem) = item.fromUid == other.item.fromUid &&
            item.toUid == other.item.toUid &&
            item.fromPid == other.item.fromPid &&
            item.packageName == other.item.packageName &&
            item.command == other.item.command &&
            item.action == other.item.action &&
            item.time == other.item.time &&
            isTop == other.isTop &&
            isBottom == other.isBottom
}
