@file:Suppress("unused")

package com.topjohnwu.magisk.ktx

import android.graphics.Canvas
import android.graphics.Rect
import android.view.View
import android.widget.EdgeEffect
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R

fun RecyclerView.addInvalidateItemDecorationsObserver() {

    adapter?.registerAdapterDataObserver(object : RecyclerView.AdapterDataObserver() {
        override fun onItemRangeInserted(positionStart: Int, itemCount: Int) {
            invalidateItemDecorations()
        }

        override fun onItemRangeRemoved(positionStart: Int, itemCount: Int) {
            invalidateItemDecorations()
        }

        override fun onItemRangeMoved(fromPosition: Int, toPosition: Int, itemCount: Int) {
            invalidateItemDecorations()
        }
    })
}

fun RecyclerView.addVerticalPadding(paddingTop: Int = 0, paddingBottom: Int = 0) {
    addItemDecoration(VerticalPaddingDecoration(paddingTop, paddingBottom))
}

private class VerticalPaddingDecoration(private val paddingTop: Int = 0, private val paddingBottom: Int = 0) : RecyclerView.ItemDecoration() {

    private var allowTop: Boolean = true
    private var allowBottom: Boolean = true

    override fun getItemOffsets(outRect: Rect, view: View, parent: RecyclerView, state: RecyclerView.State) {
        val adapter = parent.adapter ?: return
        val position = parent.getChildAdapterPosition(view)
        val count = adapter.itemCount
        if (position == 0 && allowTop) {
            outRect.top = paddingTop
        } else if (position == count - 1 && allowBottom) {
            outRect.bottom = paddingBottom
        }
    }
}

fun RecyclerView.addSimpleItemDecoration(
    left: Int = 0,
    top: Int = 0,
    right: Int = 0,
    bottom: Int = 0,
) {
    addItemDecoration(SimpleItemDecoration(left, top, right, bottom))
}

private class SimpleItemDecoration(
    private val left: Int = 0,
    private val top: Int = 0,
    private val right: Int = 0,
    private val bottom: Int = 0
) : RecyclerView.ItemDecoration() {

    private var allowLeft: Boolean = true
    private var allowTop: Boolean = true
    private var allowRight: Boolean = true
    private var allowBottom: Boolean = true

    override fun getItemOffsets(outRect: Rect, view: View, parent: RecyclerView, state: RecyclerView.State) {
        if (parent.adapter == null) {
            return
        }
        if (allowLeft) {
            outRect.left = left
        }
        if (allowTop) {
            outRect.top = top
        }
        if (allowRight) {
            outRect.right = right
        }
        if (allowBottom) {
            outRect.top = bottom
        }
    }
}

fun RecyclerView.fixEdgeEffect(overScrollIfContentScrolls: Boolean = true, alwaysClipToPadding: Boolean = true) {
    if (overScrollIfContentScrolls) {
        val listener = OverScrollIfContentScrollsListener()
        addOnLayoutChangeListener(listener)
        setTag(R.id.tag_rikka_recyclerView_OverScrollIfContentScrollsListener, listener)
    } else {
        val listener = getTag(R.id.tag_rikka_recyclerView_OverScrollIfContentScrollsListener) as? OverScrollIfContentScrollsListener
        if (listener != null) {
            removeOnLayoutChangeListener(listener)
            setTag(R.id.tag_rikka_recyclerView_OverScrollIfContentScrollsListener, null)
        }
    }

    edgeEffectFactory = if (alwaysClipToPadding && !clipToPadding) {
        AlwaysClipToPaddingEdgeEffectFactory()
    } else {
        RecyclerView.EdgeEffectFactory()
    }
}

private class OverScrollIfContentScrollsListener : View.OnLayoutChangeListener {
    private var show = true
    override fun onLayoutChange(v: View, left: Int, top: Int, right: Int, bottom: Int, oldLeft: Int, oldTop: Int, oldRight: Int, oldBottom: Int) {
        if (shouldDrawOverScroll(v as RecyclerView) != show) {
            show = !show
            if (show) {
                v.setOverScrollMode(View.OVER_SCROLL_IF_CONTENT_SCROLLS)
            } else {
                v.setOverScrollMode(View.OVER_SCROLL_NEVER)
            }
        }
    }

    fun shouldDrawOverScroll(recyclerView: RecyclerView): Boolean {
        if (recyclerView.layoutManager == null || recyclerView.adapter == null || recyclerView.adapter!!.itemCount == 0) {
            return false
        }
        if (recyclerView.layoutManager is LinearLayoutManager) {
            val itemCount = recyclerView.layoutManager!!.itemCount
            val firstPosition: Int = (recyclerView.layoutManager as LinearLayoutManager?)!!.findFirstCompletelyVisibleItemPosition()
            val lastPosition: Int = (recyclerView.layoutManager as LinearLayoutManager?)!!.findLastCompletelyVisibleItemPosition()
            return firstPosition != 0 || lastPosition != itemCount - 1
        }
        return true
    }
}

private class AlwaysClipToPaddingEdgeEffectFactory : RecyclerView.EdgeEffectFactory() {

    override fun createEdgeEffect(view: RecyclerView, direction: Int): EdgeEffect {

        return object : EdgeEffect(view.context) {
            private var ensureSize = false

            private fun ensureSize() {
                if (ensureSize) return
                ensureSize = true

                when (direction) {
                    DIRECTION_LEFT -> {
                        setSize(view.measuredHeight - view.paddingTop - view.paddingBottom,
                            view.measuredWidth - view.paddingLeft - view.paddingRight)
                    }
                    DIRECTION_TOP -> {
                        setSize(view.measuredWidth - view.paddingLeft - view.paddingRight,
                            view.measuredHeight - view.paddingTop - view.paddingBottom)
                    }
                    DIRECTION_RIGHT -> {
                        setSize(view.measuredHeight - view.paddingTop - view.paddingBottom,
                            view.measuredWidth - view.paddingLeft - view.paddingRight)
                    }
                    DIRECTION_BOTTOM -> {
                        setSize(view.measuredWidth - view.paddingLeft - view.paddingRight,
                            view.measuredHeight - view.paddingTop - view.paddingBottom)
                    }
                }
            }

            override fun draw(c: Canvas): Boolean {
                ensureSize()

                val restore = c.save()
                when (direction) {
                    DIRECTION_LEFT -> {
                        c.translate(view.paddingBottom.toFloat(), 0f)
                    }
                    DIRECTION_TOP -> {
                        c.translate(view.paddingLeft.toFloat(), view.paddingTop.toFloat())
                    }
                    DIRECTION_RIGHT -> {
                        c.translate(-view.paddingTop.toFloat(), 0f)
                    }
                    DIRECTION_BOTTOM -> {
                        c.translate(view.paddingRight.toFloat(), view.paddingBottom.toFloat())
                    }
                }
                val res = super.draw(c)
                c.restoreToCount(restore)
                return res
            }
        }
    }
}