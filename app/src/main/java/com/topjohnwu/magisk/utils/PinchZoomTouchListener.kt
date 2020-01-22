package com.topjohnwu.magisk.utils

import android.annotation.SuppressLint
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import androidx.transition.TransitionManager
import com.topjohnwu.magisk.core.Config
import kotlin.math.max
import kotlin.math.min

class PinchZoomTouchListener private constructor(
    private val view: RecyclerView,
    private val max: Int = 3,
    private val min: Int = 1
) : View.OnTouchListener {

    private val layoutManager
        get() = view.layoutManager

    private val pinchListener = object : PinchGestureCallback() {
        override fun onPinch() = updateSpanCount(Config.listSpanCount + 1)
        override fun onZoom() = updateSpanCount(Config.listSpanCount - 1)
    }

    private val gestureDetector by lazy { ScaleGestureDetector(view.context, pinchListener) }

    init {
        updateSpanCount(Config.listSpanCount, false)
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouch(v: View?, event: MotionEvent?): Boolean {
        gestureDetector.onTouchEvent(event)
        return false
    }

    private fun updateSpanCount(count: Int, animate: Boolean = true) {
        if (animate) {
            TransitionManager.beginDelayedTransition(view)
        }

        val boundCount = max(min, min(max, count))

        when (val l = layoutManager) {
            is StaggeredGridLayoutManager -> l.spanCount = boundCount
            is GridLayoutManager -> l.spanCount = boundCount
            else -> Unit
        }

        Config.listSpanCount = boundCount
    }

    companion object {

        @SuppressLint("ClickableViewAccessibility")
        fun attachTo(view: RecyclerView) = view.setOnTouchListener(PinchZoomTouchListener(view))

        fun clear(view: View) = view.setOnTouchListener(null)

    }

}
