package com.topjohnwu.magisk.model.entity.recycler

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.timeFormatMedium
import com.topjohnwu.magisk.extensions.toTime
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.magisk.model.entity.WrappedMagiskLog
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.KObservableField

class LogRvItem : ComparableRvItem<LogRvItem>() {
    override val layoutRes: Int = R.layout.item_page_log

    val items = DiffObservableList(callback)

    fun update(list: List<LogItemRvItem>) {
        list.firstOrNull()?.isExpanded?.value = true
        items.update(list)
    }

    //two of these will never be present, safe to assume it's unique
    override fun contentSameAs(other: LogRvItem): Boolean = false

    override fun itemSameAs(other: LogRvItem): Boolean = false
}

class LogItemRvItem(
    item: WrappedMagiskLog
) : ComparableRvItem<LogItemRvItem>() {
    override val layoutRes: Int = R.layout.item_superuser_log

    val date = item.time.toTime(timeFormatMedium)
    val items: List<ComparableRvItem<*>> = item.items.map { LogItemEntryRvItem(it) }
    val isExpanded = KObservableField(false)

    fun toggle() = isExpanded.toggle()

    override fun contentSameAs(other: LogItemRvItem): Boolean {
        if (items.size != other.items.size) return false
        return items.all { it in other.items }
    }

    override fun itemSameAs(other: LogItemRvItem): Boolean = date == other.date
}

class LogItemEntryRvItem(val item: MagiskLog) : ComparableRvItem<LogItemEntryRvItem>() {
    override val layoutRes: Int = R.layout.item_superuser_log_entry

    val isExpanded = KObservableField(false)

    fun toggle() = isExpanded.toggle()

    override fun contentSameAs(other: LogItemEntryRvItem) = item == other.item

    override fun itemSameAs(other: LogItemEntryRvItem) = item.appName == other.item.appName
}

class MagiskLogRvItem : ComparableRvItem<MagiskLogRvItem>() {
    override val layoutRes: Int = R.layout.item_page_magisk_log

    val items = DiffObservableList(callback)

    fun update(list: List<ConsoleRvItem>) {
        items.update(list)
    }

    //two of these will never be present, safe to assume it's unique
    override fun contentSameAs(other: MagiskLogRvItem): Boolean = false

    override fun itemSameAs(other: MagiskLogRvItem): Boolean = false
}
