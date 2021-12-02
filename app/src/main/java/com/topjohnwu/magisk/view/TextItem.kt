package com.topjohnwu.magisk.view

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.DiffRvItem

class TextItem(val text: Int) : DiffRvItem<TextItem>() {
    override val layoutRes = R.layout.item_text

    override fun contentSameAs(other: TextItem) = text == other.text
}
