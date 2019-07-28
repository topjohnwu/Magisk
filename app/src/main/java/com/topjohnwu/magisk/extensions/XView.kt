package com.topjohnwu.magisk.extensions

import android.view.View
import android.view.ViewTreeObserver

fun View.setOnViewReadyListener(callback: () -> Unit) = addOnGlobalLayoutListener(true, callback)

fun View.addOnGlobalLayoutListener(oneShot: Boolean = false, callback: () -> Unit) =
    viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
        override fun onGlobalLayout() {
            if (oneShot) viewTreeObserver.removeOnGlobalLayoutListener(this)
            callback()
        }
    })