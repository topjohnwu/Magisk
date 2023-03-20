package com.topjohnwu.magisk.ui.log

import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.textview.MaterialTextView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ObservableDiffRvItem
import com.topjohnwu.magisk.databinding.RvContainer
import com.topjohnwu.magisk.databinding.ViewAwareRvItem

class LogRvItem(
    override val item: String
) : ObservableDiffRvItem<LogRvItem>(), RvContainer<String>, ViewAwareRvItem {

    override val layoutRes = R.layout.item_log_textview

    override fun itemSameAs(other: LogRvItem) = item == other.item

    override fun onBind(binding: ViewDataBinding, recyclerView: RecyclerView) {
        val view = binding.root as MaterialTextView
        view.measure(0, 0)
        val desiredWidth = view.measuredWidth
        val layoutParams = view.layoutParams
        layoutParams.width = desiredWidth
        if (recyclerView.width < desiredWidth) {
            recyclerView.requestLayout()
        }
    }
}
