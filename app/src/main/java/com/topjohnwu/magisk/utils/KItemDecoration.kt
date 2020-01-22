package com.topjohnwu.magisk.utils

import android.content.Context
import android.graphics.Canvas
import android.graphics.Rect
import android.graphics.drawable.Drawable
import android.view.View
import androidx.annotation.DrawableRes
import androidx.core.view.get
import androidx.recyclerview.widget.DividerItemDecoration
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.extensions.drawableCompat
import kotlin.math.roundToInt

class KItemDecoration(
    private val context: Context,
    @RecyclerView.Orientation private val orientation: Int
) :
    RecyclerView.ItemDecoration() {

    private val bounds = Rect()
    private var divider: Drawable? = null
    var showAfterLast = true

    fun setDeco(@DrawableRes drawable: Int) = apply {
        setDeco(context.drawableCompat(drawable))
    }

    fun setDeco(drawable: Drawable?) = apply {
        divider = drawable
    }

    override fun onDraw(canvas: Canvas, parent: RecyclerView, state: RecyclerView.State) {
        parent.layoutManager ?: return

        divider?.let {
            if (orientation == DividerItemDecoration.VERTICAL) {
                drawVertical(canvas, parent, it)
            } else {
                drawHorizontal(canvas, parent, it)
            }
        }
    }

    private fun drawVertical(canvas: Canvas, parent: RecyclerView, drawable: Drawable) {
        canvas.save()
        val left: Int
        val right: Int
        if (parent.clipToPadding) {
            left = parent.paddingLeft
            right = parent.width - parent.paddingRight
            canvas.clipRect(
                left, parent.paddingTop, right,
                parent.height - parent.paddingBottom
            )
        } else {
            left = 0
            right = parent.width
        }

        val to = if (showAfterLast) parent.childCount else parent.childCount - 1

        (0 until to)
            .map { parent[it] }
            .forEach { child ->
                parent.getDecoratedBoundsWithMargins(child, bounds)
                val bottom = bounds.bottom + child.translationY.roundToInt()
                val top = bottom - drawable.intrinsicHeight
                drawable.setBounds(left, top, right, bottom)
                drawable.draw(canvas)
            }
        canvas.restore()
    }

    private fun drawHorizontal(canvas: Canvas, parent: RecyclerView, drawable: Drawable) {
        canvas.save()
        val top: Int
        val bottom: Int
        if (parent.clipToPadding) {
            top = parent.paddingTop
            bottom = parent.height - parent.paddingBottom
            canvas.clipRect(
                parent.paddingLeft, top,
                parent.width - parent.paddingRight, bottom
            )
        } else {
            top = 0
            bottom = parent.height
        }

        val to = if (showAfterLast) parent.childCount else parent.childCount - 1

        (0 until to)
            .map { parent[it] }
            .forEach { child ->
                parent.layoutManager!!.getDecoratedBoundsWithMargins(child, bounds)
                val right = bounds.right + child.translationX.roundToInt()
                val left = right - drawable.intrinsicWidth
                drawable.setBounds(left, top, right, bottom)
                drawable.draw(canvas)
            }
        canvas.restore()
    }

    override fun getItemOffsets(outRect: Rect, view: View, parent: RecyclerView, state: RecyclerView.State) {
        if (parent.getChildAdapterPosition(view) == state.itemCount - 1) {
            outRect.setEmpty()
            return
        }

        if (orientation == RecyclerView.VERTICAL) {
            outRect.set(0, 0, 0, divider?.intrinsicHeight ?: 0)
        } else {
            outRect.set(0, 0, divider?.intrinsicWidth ?: 0, 0)
        }
    }

}
