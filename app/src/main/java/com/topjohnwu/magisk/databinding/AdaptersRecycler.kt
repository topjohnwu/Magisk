package com.topjohnwu.magisk.databinding

import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.InsetDrawable
import androidx.databinding.BindingAdapter
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.extensions.startEndToLeftRight
import com.topjohnwu.magisk.extensions.toPx
import com.topjohnwu.magisk.utils.KItemDecoration
import kotlin.math.roundToInt

@BindingAdapter(
    "dividerColor",
    "dividerHorizontal",
    "dividerSize",
    "dividerAfterLast",
    "dividerMarginStart",
    "dividerMarginEnd",
    "dividerMarginTop",
    "dividerMarginBottom",
    requireAll = false
)
fun setDivider(
    view: RecyclerView,
    color: Int,
    horizontal: Boolean,
    _size: Float,
    _afterLast: Boolean?,
    marginStartF: Float,
    marginEndF: Float,
    marginTopF: Float,
    marginBottomF: Float
) {
    val orientation = if (horizontal) RecyclerView.HORIZONTAL else RecyclerView.VERTICAL
    val size = if (_size > 0) _size.roundToInt() else 1.toPx()
    val (width, height) = if (horizontal) size to 1 else 1 to size
    val afterLast = _afterLast ?: true

    val marginStart = marginStartF.roundToInt()
    val marginEnd = marginEndF.roundToInt()
    val marginTop = marginTopF.roundToInt()
    val marginBottom = marginBottomF.roundToInt()
    val (marginLeft, marginRight) = view.context.startEndToLeftRight(marginStart, marginEnd)

    val drawable = GradientDrawable().apply {
        setSize(width, height)
        shape = GradientDrawable.RECTANGLE
        setColor(color)
    }.let {
        InsetDrawable(it, marginLeft, marginTop, marginRight, marginBottom)
    }

    val decoration = KItemDecoration(view.context, orientation)
        .setDeco(drawable)
        .apply { showAfterLast = afterLast }
    view.addItemDecoration(decoration)
}
