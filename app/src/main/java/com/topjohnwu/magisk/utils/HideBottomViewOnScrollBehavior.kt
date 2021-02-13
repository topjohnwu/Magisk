package com.topjohnwu.magisk.utils

import android.content.Context
import android.util.AttributeSet
import android.view.View
import android.view.ViewGroup
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.view.isGone
import androidx.core.view.isVisible
import androidx.core.view.updateLayoutParams
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.google.android.material.behavior.HideBottomViewOnScrollBehavior
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import kotlin.math.roundToInt

class HideBottomViewOnScrollBehavior<V : View>(context: Context, attrs: AttributeSet) :
    HideBottomViewOnScrollBehavior<V>(), HideableBehavior<V> {

    private var lockState: Boolean = false
    private var isLaidOut = false

    override fun layoutDependsOn(parent: CoordinatorLayout, child: V, dependency: View) =
        super.layoutDependsOn(parent, child, dependency) or (dependency is Snackbar.SnackbarLayout)

    override fun onLayoutChild(parent: CoordinatorLayout, child: V, layoutDirection: Int): Boolean {
        isLaidOut = true
        return super.onLayoutChild(parent, child, layoutDirection)
    }

    override fun onDependentViewChanged(
        parent: CoordinatorLayout,
        child: V,
        dependency: View
    ) = when (dependency) {
        is Snackbar.SnackbarLayout -> onDependentViewChanged(parent, child, dependency)
        else -> super.onDependentViewChanged(parent, child, dependency)
    }

    override fun onDependentViewRemoved(
        parent: CoordinatorLayout,
        child: V,
        dependency: View
    ) = when (dependency) {
        is Snackbar.SnackbarLayout -> onDependentViewRemoved(parent, child, dependency)
        else -> super.onDependentViewRemoved(parent, child, dependency)
    }

    //---

    private fun onDependentViewChanged(
        parent: CoordinatorLayout,
        child: V,
        dependency: Snackbar.SnackbarLayout
    ): Boolean {
        val viewMargin = (child.layoutParams as ViewGroup.MarginLayoutParams).bottomMargin
        val additionalMargin = dependency.resources.getDimension(R.dimen.l1).roundToInt()
        val translation = dependency.height + additionalMargin

        dependency.updateLayoutParams<ViewGroup.MarginLayoutParams> {
            bottomMargin = viewMargin
        }

        // checks whether the navigation is not hidden via scroll
        if (child.isVisible && child.translationY <= 0) {
            child.translationY(-translation.toFloat())
        }
        return false
    }

    private fun onDependentViewRemoved(
        parent: CoordinatorLayout,
        child: V,
        dependency: Snackbar.SnackbarLayout
    ) {
        // checks whether the navigation is not hidden via scroll
        if (child.isVisible && child.translationY <= 0) {
            child.translationY(0f)
        }
    }

    //---

    override fun slideUp(child: V) {
        if (lockState) return
        super.slideUp(child)
    }

    override fun slideDown(child: V) {
        if (lockState) return
        super.slideDown(child)
    }

    override fun setHidden(
        view: V,
        hide: Boolean,
        lockState: Boolean
    ) {
        if (!lockState) {
            this.lockState = lockState
        }

        if (hide || !Info.env.isActive) {
            // view is not laid out and drawn yet properly, so animation will not be attached
            // hence we just simply hide the view
            if (!isLaidOut) {
                view.isGone = true
            } else {
                slideDown(view)
            }
        } else {
            view.isVisible = Info.env.isActive
            slideUp(view)
        }

        this.lockState = lockState
    }

    //---

    private fun View.translationY(destination: Float) = animate()
        .translationY(destination)
        .setInterpolator(FastOutSlowInInterpolator())
        .start()

}
