package com.topjohnwu.magisk.model.entity.recycler

import com.skoumal.teanity.databinding.ComparableRvItem
import com.topjohnwu.magisk.R

class ConsoleRvItem(val item: String) : ComparableRvItem<ConsoleRvItem>() {
    override val layoutRes: Int = R.layout.item_console

    override fun contentSameAs(other: ConsoleRvItem) = itemSameAs(other)
    override fun itemSameAs(other: ConsoleRvItem) = item == other.item
}