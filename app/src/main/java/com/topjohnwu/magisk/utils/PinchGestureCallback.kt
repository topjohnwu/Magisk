package com.topjohnwu.magisk.utils

import android.view.ScaleGestureDetector

abstract class PinchGestureCallback : ScaleGestureDetector.SimpleOnScaleGestureListener() {

    private var startFactor: Float = 1f

    override fun onScaleBegin(detector: ScaleGestureDetector?): Boolean {
        startFactor = detector?.scaleFactor ?: 1f
        return super.onScaleBegin(detector)
    }

    override fun onScaleEnd(detector: ScaleGestureDetector?) {
        val endFactor = detector?.scaleFactor ?: 1f

        if (endFactor > startFactor) onZoom()
        else if (endFactor < startFactor) onPinch()
    }

    abstract fun onPinch()
    abstract fun onZoom()

}