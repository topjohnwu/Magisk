package com.topjohnwu.magisk.utils

import android.animation.TimeInterpolator
import android.view.View
import android.view.ViewGroup
import android.view.ViewPropertyAnimator
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.view.ViewCompat
import com.google.android.material.animation.AnimationUtils

class HideTopViewOnScrollBehavior<V : View> :
    CoordinatorLayout.Behavior<V>(),
    HideableBehavior<V> {

    companion object {
        private const val STATE_SCROLLED_DOWN = 1
        private const val STATE_SCROLLED_UP = 2

        private const val ENTER_ANIMATION_DURATION = 225
        private const val EXIT_ANIMATION_DURATION = 175
    }

    private var height = 0
    private var currentState = STATE_SCROLLED_UP
    private var currentAnimator: ViewPropertyAnimator? = null
    private var lockState: Boolean = false

    override fun onLayoutChild(
        parent: CoordinatorLayout,
        child: V,
        layoutDirection: Int
    ): Boolean {
        val paramsCompat = child.layoutParams as ViewGroup.MarginLayoutParams
        height = child.measuredHeight + paramsCompat.topMargin
        return super.onLayoutChild(parent, child, layoutDirection)
    }

    override fun onStartNestedScroll(
        coordinatorLayout: CoordinatorLayout,
        child: V,
        directTargetChild: View,
        target: View,
        nestedScrollAxes: Int,
        type: Int
    ) = nestedScrollAxes == ViewCompat.SCROLL_AXIS_VERTICAL

    override fun onNestedScroll(
        coordinatorLayout: CoordinatorLayout,
        child: V,
        target: View,
        dxConsumed: Int,
        dyConsumed: Int,
        dxUnconsumed: Int,
        dyUnconsumed: Int,
        type: Int,
        consumed: IntArray
    ) {
        // when initiating scroll while the view is at the bottom or at the top and pushing it
        // further, the parent will report consumption of 0
        if (dyConsumed == 0) return

        setHidden(child, dyConsumed > 0, false)
    }

    @Suppress("UNCHECKED_CAST")
    override fun setHidden(
        view: V,
        hide: Boolean,
        lockState: Boolean
    ) {
        if (!lockState) {
            this.lockState = lockState
        }

        if (hide) {
            slideUp(view)
        } else {
            slideDown(view)
        }

        this.lockState = lockState
    }

    /**
     * Perform an animation that will slide the child from it's current position to be totally on the
     * screen.
     */
    private fun slideDown(child: V) {
        if (currentState == STATE_SCROLLED_UP || lockState) {
            return
        }

        currentAnimator?.let {
            it.cancel()
            child.clearAnimation()
        }

        currentState = STATE_SCROLLED_UP
        animateChildTo(
            child,
            0,
            ENTER_ANIMATION_DURATION.toLong(),
            AnimationUtils.LINEAR_OUT_SLOW_IN_INTERPOLATOR
        )
    }

    /**
     * Perform an animation that will slide the child from it's current position to be totally off the
     * screen.
     */
    private fun slideUp(child: V) {
        if (currentState == STATE_SCROLLED_DOWN || lockState) {
            return
        }

        currentAnimator?.let {
            it.cancel()
            child.clearAnimation()
        }

        currentState = STATE_SCROLLED_DOWN
        animateChildTo(
            child,
            -height,
            EXIT_ANIMATION_DURATION.toLong(),
            AnimationUtils.FAST_OUT_LINEAR_IN_INTERPOLATOR
        )
    }

    private fun animateChildTo(
        child: V,
        targetY: Int,
        duration: Long,
        interpolator: TimeInterpolator
    ) = child
        .animate()
        .translationY(targetY.toFloat())
        .setInterpolator(interpolator)
        .setDuration(duration)
        .withEndAction { currentAnimator = null }
        .let { currentAnimator = it }
}