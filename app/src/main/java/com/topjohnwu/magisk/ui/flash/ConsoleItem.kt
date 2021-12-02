package com.topjohnwu.magisk.ui.flash

import android.view.View
import android.widget.TextView
import androidx.core.view.updateLayoutParams
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.DiffRvItem
import com.topjohnwu.magisk.databinding.LenientRvItem
import com.topjohnwu.magisk.databinding.RvContainer
import kotlin.math.max

class ConsoleItem(
    override val item: String
) : DiffRvItem<ConsoleItem>(), LenientRvItem,
    RvContainer<String> {
    override val layoutRes = R.layout.item_console_md2

    private var parentWidth = -1

    override fun onBindingBound(binding: ViewDataBinding, recyclerView: RecyclerView) {
        if (parentWidth < 0)
            parentWidth = (recyclerView.parent as View).width

        val view = binding.root as TextView
        view.measure(0, 0)

        // We want our recyclerView at least as wide as screen
        val desiredWidth = max(view.measuredWidth, parentWidth)

        view.updateLayoutParams { width = desiredWidth }

        if (recyclerView.width < desiredWidth) {
            recyclerView.requestLayout()
        }
    }
}
