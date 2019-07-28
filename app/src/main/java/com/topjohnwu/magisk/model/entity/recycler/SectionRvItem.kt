package com.topjohnwu.magisk.model.entity.recycler

import com.skoumal.teanity.databinding.ComparableRvItem
import com.topjohnwu.magisk.R

class SectionRvItem(val text: String) : ComparableRvItem<SectionRvItem>() {
    override val layoutRes: Int = R.layout.item_section

    override fun contentSameAs(other: SectionRvItem) = itemSameAs(other)
    override fun itemSameAs(other: SectionRvItem) = text == other.text
}