package com.topjohnwu.magisk.extensions

import android.view.View
import android.view.ViewGroup
import android.view.ViewTreeObserver
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import androidx.transition.AutoTransition
import androidx.transition.TransitionManager

fun View.setOnViewReadyListener(callback: () -> Unit) = addOnGlobalLayoutListener(true, callback)

fun View.addOnGlobalLayoutListener(oneShot: Boolean = false, callback: () -> Unit) =
    viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
        override fun onGlobalLayout() {
            if (oneShot) viewTreeObserver.removeOnGlobalLayoutListener(this)
            callback()
        }
    })

fun ViewGroup.startAnimations() {
    val transition = AutoTransition().setInterpolator(FastOutSlowInInterpolator()).setDuration(400)
    TransitionManager.beginDelayedTransition(this, transition)
}