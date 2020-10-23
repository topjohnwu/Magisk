@file:Suppress("unused")

package com.topjohnwu.magisk.ui.inflater

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.graphics.Rect
import android.os.Build
import android.util.AttributeSet
import android.view.Gravity.*
import android.view.View
import android.view.ViewGroup
import androidx.core.graphics.Insets
import androidx.core.view.OnApplyWindowInsetsListener
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.topjohnwu.magisk.R

private typealias ApplyInsetsCallback<T> = (insets: Insets, left: Boolean, top: Boolean, right: Boolean, bottom: Boolean) -> T

private class ApplyInsets(private val out: Rect) : ApplyInsetsCallback<Unit> {

    override fun invoke(insets: Insets, left: Boolean, top: Boolean, right: Boolean, bottom: Boolean) {
        out.left += if (left) insets.left else 0
        out.top += if (top) insets.top else 0
        out.right += if (right) insets.right else 0
        out.bottom += if (bottom) insets.bottom else 0
    }
}

private class ConsumeInsets : ApplyInsetsCallback<Insets> {

    override fun invoke(insets: Insets, left: Boolean, top: Boolean, right: Boolean, bottom: Boolean): Insets {
        val insetsLeft = if (left) 0 else insets.left
        val insetsTop = if (top) 0 else insets.top
        val insetsRight = if (right) 0 else insets.right
        val insetsBottom = if (bottom) 0 else insets.bottom
        return Insets.of(insetsLeft, insetsTop, insetsRight, insetsBottom)
    }
}

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
open class WindowInsetsHelper private constructor(
        private val view: View,
        private val fitSystemWindows: Int,
        private val layout_fitsSystemWindowsInsets: Int,
        private val consumeSystemWindows: Int) : OnApplyWindowInsetsListener {

    internal var initialPaddingLeft: Int = view.paddingLeft
    internal var initialPaddingTop: Int = view.paddingTop
    internal var initialPaddingRight: Int = view.paddingRight
    internal var initialPaddingBottom: Int = view.paddingBottom

    private var initialMargin = false
    internal var initialMarginLeft: Int = 0
    internal var initialMarginTop: Int = 0
    internal var initialMarginRight: Int = 0
    internal var initialMarginBottom: Int = 0
    internal var initialMarginStart: Int = 0
    internal var initialMarginEnd: Int = 0

    private var lastInsets: WindowInsetsCompat? = null

    open fun setInitialPadding(left: Int, top: Int, right: Int, bottom: Int) {
        initialPaddingLeft = left
        initialPaddingTop = top
        initialPaddingRight = right
        initialPaddingBottom = bottom

        lastInsets?.let { applyWindowInsets(it) }
    }

    open fun setInitialPaddingRelative(start: Int, top: Int, end: Int, bottom: Int) {
        val isRTL = view.layoutDirection == View.LAYOUT_DIRECTION_RTL
        if (isRTL) {
            setInitialPadding(start, top, end, bottom)
        } else {
            setInitialPadding(start, top, end, bottom)
        }
    }

    open fun setInitialMargin(left: Int, top: Int, right: Int, bottom: Int) {
        initialPaddingLeft = left
        initialPaddingTop = top
        initialPaddingRight = right
        initialPaddingBottom = bottom

        lastInsets?.let { applyWindowInsets(it) }
    }

    open fun setInitialMarginRelative(start: Int, top: Int, end: Int, bottom: Int) {
        initialMarginStart = start
        initialMarginTop = top
        initialMarginEnd = end
        initialMarginBottom = bottom

        lastInsets?.let { applyWindowInsets(it) }
    }

    @SuppressLint("RtlHardcoded")
    private fun <T> applyInsets(insets: Insets, fit: Int, callback: ApplyInsetsCallback<T>): T {
        val relativeMode = (fit and RELATIVE_LAYOUT_DIRECTION) == RELATIVE_LAYOUT_DIRECTION

        val isRTL = view.layoutDirection == View.LAYOUT_DIRECTION_RTL

        val left: Boolean
        val top = fit and TOP == TOP
        val right: Boolean
        val bottom = fit and BOTTOM == BOTTOM

        if (relativeMode) {
            val start = fit and START == START
            val end = fit and END == END
            left = (!isRTL && start) || (isRTL && end)
            right = (!isRTL && end) || (isRTL && start)
        } else {
            left = fit and LEFT == LEFT
            right = fit and RIGHT == RIGHT
        }

        return callback.invoke(insets, left, top, right, bottom)
    }

    private fun applyWindowInsets(windowInsets: WindowInsetsCompat): WindowInsetsCompat {
        if (fitSystemWindows != 0) {
            val padding = Rect(initialPaddingLeft, initialPaddingTop, initialPaddingRight, initialPaddingBottom)
            applyInsets(windowInsets.systemWindowInsets, fitSystemWindows, ApplyInsets(padding))
            view.setPadding(padding.left, padding.top, padding.right, padding.bottom)
        }

        if (layout_fitsSystemWindowsInsets != 0) {
            if (!initialMargin) {
                initialMarginLeft = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.leftMargin ?: 0
                initialMarginTop = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.topMargin ?: 0
                initialMarginRight = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.rightMargin ?: 0
                initialMarginBottom = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.bottomMargin ?: 0
                initialMarginStart = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.marginStart ?: 0
                initialMarginEnd = (view.layoutParams as? ViewGroup.MarginLayoutParams)?.marginEnd ?: 0
                initialMargin = true
            }

            val margin = if ((layout_fitsSystemWindowsInsets and RELATIVE_LAYOUT_DIRECTION) == RELATIVE_LAYOUT_DIRECTION)
                Rect(initialMarginLeft, initialMarginTop, initialMarginRight, initialMarginBottom)
            else
                Rect(initialMarginStart, initialMarginTop, initialMarginEnd, initialMarginBottom)

            applyInsets(windowInsets.systemWindowInsets, layout_fitsSystemWindowsInsets, ApplyInsets(margin))

            val lp = view.layoutParams
            if (lp is ViewGroup.MarginLayoutParams) {
                lp.topMargin = margin.top
                lp.bottomMargin = margin.bottom

                if ((layout_fitsSystemWindowsInsets and RELATIVE_LAYOUT_DIRECTION) == RELATIVE_LAYOUT_DIRECTION) {
                    lp.marginStart = margin.left
                    lp.marginEnd = margin.right
                } else {
                    lp.leftMargin = margin.left
                    lp.rightMargin = margin.right
                }

                view.layoutParams = lp
            }
        }

        val systemWindowInsets = if (consumeSystemWindows != 0) applyInsets(windowInsets.systemWindowInsets, consumeSystemWindows, ConsumeInsets()) else windowInsets.systemWindowInsets

        return WindowInsetsCompat.Builder(windowInsets)
                .setSystemWindowInsets(systemWindowInsets)
                .build()
    }

    override fun onApplyWindowInsets(view: View, insets: WindowInsetsCompat): WindowInsetsCompat {
        if (lastInsets == insets) {
            return insets
        }

        lastInsets = insets

        return applyWindowInsets(insets)
    }

    companion object {

        @JvmStatic
        fun attach(view: View, attrs: AttributeSet) {
            val a = view.context.obtainStyledAttributes(attrs, R.styleable.WindowInsetsHelper, 0, 0)
            val edgeToEdge = a.getBoolean(R.styleable.WindowInsetsHelper_edgeToEdge, false)
            val fitsSystemWindowsInsets = a.getInt(R.styleable.WindowInsetsHelper_fitsSystemWindowsInsets, 0)
            val layout_fitsSystemWindowsInsets = a.getInt(R.styleable.WindowInsetsHelper_layout_fitsSystemWindowsInsets, 0)
            val consumeSystemWindowsInsets = a.getInt(R.styleable.WindowInsetsHelper_consumeSystemWindowsInsets, 0)
            a.recycle()

            attach(view, edgeToEdge, fitsSystemWindowsInsets, layout_fitsSystemWindowsInsets, consumeSystemWindowsInsets)
        }

        @JvmStatic
        fun attach(view: View, edgeToEdge: Boolean, fitsSystemWindowsInsets: Int, layout_fitsSystemWindowsInsets: Int, consumeSystemWindowsInsets: Int) {
            if (edgeToEdge) {
                view.systemUiVisibility = (view.systemUiVisibility
                        or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION)
            }

            if (fitsSystemWindowsInsets == 0 && layout_fitsSystemWindowsInsets == 0 && consumeSystemWindowsInsets == 0) {
                return
            }

            val listener = WindowInsetsHelper(view, fitsSystemWindowsInsets, layout_fitsSystemWindowsInsets, consumeSystemWindowsInsets)
            ViewCompat.setOnApplyWindowInsetsListener(view, listener)
            view.setTag(R.id.tag_rikka_material_WindowInsetsHelper, listener)

            if (!view.isAttachedToWindow) {
                view.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {
                    override fun onViewAttachedToWindow(v: View) {
                        v.removeOnAttachStateChangeListener(this)
                        v.requestApplyInsets()
                    }

                    override fun onViewDetachedFromWindow(v: View) = Unit
                })
            }
        }
    }
}

val View.windowInsetsHelper: WindowInsetsHelper?
    get() {
        val value = getTag(R.id.tag_rikka_material_WindowInsetsHelper)
        return if (value is WindowInsetsHelper) value else null
    }

val View.initialPaddingLeft: Int
    get() = windowInsetsHelper?.initialPaddingLeft ?: 0

val View.initialPaddingTop: Int
    get() = windowInsetsHelper?.initialPaddingTop ?: 0

val View.initialPaddingRight: Int
    get() = windowInsetsHelper?.initialPaddingRight ?: 0

val View.initialPaddingBottom: Int
    get() = windowInsetsHelper?.initialPaddingBottom ?: 0

val View.initialPaddingStart: Int
    get() = if (layoutDirection == View.LAYOUT_DIRECTION_RTL) initialPaddingRight else initialPaddingLeft

val View.initialPaddingEnd: Int
    get() = if (layoutDirection == View.LAYOUT_DIRECTION_RTL) initialPaddingLeft else initialPaddingRight

fun View.setInitialPadding(left: Int, top: Int, right: Int, bottom: Int) {
    windowInsetsHelper?.setInitialPadding(left, top, right, bottom)
}

fun View.setInitialPaddingRelative(start: Int, top: Int, end: Int, bottom: Int) {
    windowInsetsHelper?.setInitialPaddingRelative(start, top, end, bottom)
}

val View.initialMarginLeft: Int
    get() = windowInsetsHelper?.initialMarginLeft ?: 0

val View.initialMarginTop: Int
    get() = windowInsetsHelper?.initialMarginTop ?: 0

val View.initialMarginRight: Int
    get() = windowInsetsHelper?.initialMarginRight ?: 0

val View.initialMarginBottom: Int
    get() = windowInsetsHelper?.initialMarginBottom ?: 0

val View.initialMarginStart: Int
    get() = windowInsetsHelper?.initialMarginStart ?: 0

val View.initialMarginEnd: Int
    get() = windowInsetsHelper?.initialMarginEnd ?: 0

fun View.setInitialMargin(left: Int, top: Int, right: Int, bottom: Int) {
    windowInsetsHelper?.setInitialMargin(left, top, right, bottom)
}

fun View.setInitialMarginRelative(start: Int, top: Int, end: Int, bottom: Int) {
    windowInsetsHelper?.setInitialMarginRelative(start, top, end, bottom)
}