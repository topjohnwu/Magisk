package com.topjohnwu.magisk.model.entity.recycler

import com.skoumal.teanity.databinding.ComparableRvItem
import com.topjohnwu.magisk.R

class SpinnerRvItem(val item: String) : ComparableRvItem<SpinnerRvItem>() {

    override val layoutRes: Int = R.layout.item_spinner

    override fun contentSameAs(other: SpinnerRvItem) = itemSameAs(other)
    override fun itemSameAs(other: SpinnerRvItem) = item == other.item

}