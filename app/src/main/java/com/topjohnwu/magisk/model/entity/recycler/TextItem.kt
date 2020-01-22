package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ComparableRvItem

class TextItem(val text: Int) : ComparableRvItem<TextItem>() {
    override val layoutRes = R.layout.item_text

    override fun onBindingBound(binding: ViewDataBinding) {
        super.onBindingBound(binding)
        val params = binding.root.layoutParams as? StaggeredGridLayoutManager.LayoutParams
        params?.isFullSpan = true
    }

    override fun contentSameAs(other: TextItem) = text == other.text
    override fun itemSameAs(other: TextItem) = contentSameAs(other)
}