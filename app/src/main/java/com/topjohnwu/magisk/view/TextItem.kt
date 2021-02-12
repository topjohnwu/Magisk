package com.topjohnwu.magisk.view

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem

class TextItem(val text: Int) : ComparableRvItem<TextItem>() {
    override val layoutRes = R.layout.item_text

    override fun contentSameAs(other: TextItem) = text == other.text
    override fun itemSameAs(other: TextItem) = contentSameAs(other)
}
