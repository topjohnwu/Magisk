package com.topjohnwu.magisk.ui.log

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.databinding.ObservableDiffRvItem
import com.topjohnwu.magisk.databinding.RvContainer
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.ktx.timeDateFormat
import com.topjohnwu.magisk.ktx.toTime

class LogRvItem(
    override val item: SuLog
) : ObservableDiffRvItem<LogRvItem>(), RvContainer<SuLog> {

    override val layoutRes = R.layout.item_log_access_md2

    val date = item.time.toTime(timeDateFormat)

    @get:Bindable
    var isTop = false
        set(value) = set(value, field, { field = it }, BR.top)

    @get:Bindable
    var isBottom = false
        set(value) = set(value, field, { field = it }, BR.bottom)

    override fun itemSameAs(other: LogRvItem) = item.appName == other.item.appName
}
