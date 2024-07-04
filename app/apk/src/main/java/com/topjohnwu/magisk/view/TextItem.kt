package com.topjohnwu.magisk.view

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.DiffItem
import com.topjohnwu.magisk.databinding.ItemWrapper
import com.topjohnwu.magisk.databinding.RvItem

class TextItem(override val item: Int) : RvItem(), DiffItem<TextItem>, ItemWrapper<Int> {
    override val layoutRes = R.layout.item_text
}
