package com.topjohnwu.magisk.model.entity.recycler

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem

class SpinnerRvItem(val item: String) : ComparableRvItem<SpinnerRvItem>() {

    override val layoutRes: Int = R.layout.item_spinner

    override fun contentSameAs(other: SpinnerRvItem) = itemSameAs(other)
    override fun itemSameAs(other: SpinnerRvItem) = item == other.item

}