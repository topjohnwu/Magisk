package com.topjohnwu.magisk.model.entity.recycler

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.ui.theme.Theme

class ThemeItem(val theme: Theme) : ComparableRvItem<ThemeItem>() {

    override val layoutRes = R.layout.item_theme

    override fun contentSameAs(other: ThemeItem) = itemSameAs(other)
    override fun itemSameAs(other: ThemeItem) = theme == other.theme

}
