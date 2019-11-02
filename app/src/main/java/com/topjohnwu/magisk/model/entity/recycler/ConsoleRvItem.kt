package com.topjohnwu.magisk.model.entity.recycler

import android.widget.TextView
import androidx.core.view.updateLayoutParams
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R

class ConsoleRvItem(val item: String) : LenientRvItem<ConsoleRvItem>() {
    override val layoutRes: Int = R.layout.item_console

    override fun onBindingBound(binding: ViewDataBinding, recyclerView: RecyclerView) {
        val view = binding.root as TextView
        view.measure(0, 0)
        val desiredWidth = view.measuredWidth

        view.updateLayoutParams { width = desiredWidth }

        if (recyclerView.width < desiredWidth) {
            recyclerView.requestLayout()
        }
    }

    override fun contentSameAs(other: ConsoleRvItem) = itemSameAs(other)
    override fun itemSameAs(other: ConsoleRvItem) = item == other.item
}