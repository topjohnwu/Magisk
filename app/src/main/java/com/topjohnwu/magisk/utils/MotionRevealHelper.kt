package com.topjohnwu.magisk.utils

import android.animation.Animator
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.view.View
import androidx.core.animation.addListener
import androidx.core.text.layoutDirection
import androidx.core.view.isInvisible
import androidx.core.view.isVisible
import androidx.core.view.marginBottom
import androidx.core.view.marginEnd
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.google.android.material.circularreveal.CircularRevealCompat
import com.google.android.material.circularreveal.CircularRevealWidget
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.topjohnwu.magisk.core.utils.currentLocale
import kotlin.math.hypot

object MotionRevealHelper {

    fun <CV> withViews(
        revealable: CV,
        fab: FloatingActionButton,
        expanded: Boolean
    ) where CV : CircularRevealWidget, CV : View {
        revealable.revealInfo = revealable.createRevealInfo(!expanded)

        val revealInfo = revealable.createRevealInfo(expanded)
        val revealAnim = revealable.createRevealAnim(revealInfo)
        val moveAnim = fab.createMoveAnim(revealInfo)

        AnimatorSet().also {
            if (expanded) {
                it.play(revealAnim).after(moveAnim)
            } else {
                it.play(moveAnim).after(revealAnim)
            }
        }.start()
    }

    private fun <CV> CV.createRevealAnim(
        revealInfo: CircularRevealWidget.RevealInfo
    ): Animator where CV : CircularRevealWidget, CV : View =
        CircularRevealCompat.createCircularReveal(
            this,
            revealInfo.centerX,
            revealInfo.centerY,
            revealInfo.radius
        ).apply {
            addListener(onStart = {
                isVisible = true
            }, onEnd = {
                if (revealInfo.radius == 0f) {
                    isInvisible = true
                }
            })
        }

    private fun FloatingActionButton.createMoveAnim(
        revealInfo: CircularRevealWidget.RevealInfo
    ): Animator = AnimatorSet().also {
        it.interpolator = FastOutSlowInInterpolator()
        it.addListener(onStart = { show() }, onEnd = { if (revealInfo.radius != 0f) hide() })

        val rtlMod = if (currentLocale.layoutDirection == View.LAYOUT_DIRECTION_RTL) 1f else -1f
        val maxX = revealInfo.centerX - marginEnd - measuredWidth / 2f
        val targetX = if (revealInfo.radius == 0f) 0f else maxX * rtlMod
        val moveX = ObjectAnimator.ofFloat(this, View.TRANSLATION_X, targetX)

        val maxY = revealInfo.centerY - marginBottom - measuredHeight / 2f
        val targetY = if (revealInfo.radius == 0f) 0f else -maxY
        val moveY = ObjectAnimator.ofFloat(this, View.TRANSLATION_Y, targetY)

        it.playTogether(moveX, moveY)
    }

    private fun View.createRevealInfo(expanded: Boolean): CircularRevealWidget.RevealInfo {
        val cX = measuredWidth / 2f
        val cY = measuredHeight / 2f - paddingBottom
        return CircularRevealWidget.RevealInfo(cX, cY, if (expanded) hypot(cX, cY) else 0f)
    }

}
