package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.ObservableList
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.SuLogEntry
import com.topjohnwu.magisk.utils.toggle

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
    val items: ObservableList<ComparableRvItem<*>>
) : ComparableRvItem<LogItemRvItem>() {
    override val layoutRes: Int = R.layout.item_superuser_log

    val date = items.filterIsInstance<LogItemEntryRvItem>().firstOrNull()
        ?.item?.dateString.orEmpty()
    val isExpanded = KObservableField(false)

    fun toggle() = isExpanded.toggle()

    override fun contentSameAs(other: LogItemRvItem): Boolean = items
        .any { !other.items.contains(it) }

    override fun itemSameAs(other: LogItemRvItem): Boolean = date == other.date
}

class LogItemEntryRvItem(val item: SuLogEntry) : ComparableRvItem<LogItemEntryRvItem>() {
    override val layoutRes: Int = R.layout.item_superuser_log_entry

    val isExpanded = KObservableField(false)

    fun toggle() = isExpanded.toggle()

    override fun contentSameAs(other: LogItemEntryRvItem) = item.fromUid == other.item.fromUid &&
            item.toUid == other.item.toUid &&
            item.fromPid == other.item.fromPid &&
            item.packageName == other.item.packageName &&
            item.command == other.item.command &&
            item.action == other.item.action &&
            item.date == other.item.date

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