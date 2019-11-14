package com.topjohnwu.magisk.redesign.hide

import android.animation.Animator
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.content.Context
import android.graphics.Insets
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.animation.addListener
import androidx.core.view.isInvisible
import androidx.core.view.isVisible
import androidx.core.view.marginBottom
import androidx.core.view.marginEnd
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.google.android.material.circularreveal.CircularRevealCompat
import com.google.android.material.circularreveal.CircularRevealWidget
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHideMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import com.topjohnwu.magisk.redesign.compat.hideKeyboard
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.math.hypot

class HideFragment : CompatFragment<HideViewModel, FragmentHideMd2Binding>() {

    override val layoutRes = R.layout.fragment_hide_md2
    override val viewModel by viewModel<HideViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onAttach(context: Context) {
        super.onAttach(context)
        activity.setTitle(R.string.magiskhide)
        setHasOptionsMenu(true)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.hideFilterToggle.setOnClickListener {
            MotionRevealHelper.withViews(binding.hideFilter, binding.hideFilterToggle, true)
        }
        binding.hideFilterInclude.hideFilterDone.setOnClickListener {
            hideKeyboard()
            MotionRevealHelper.withViews(binding.hideFilter, binding.hideFilterToggle, false)
        }

        val lama = binding.hideContent.layoutManager ?: return
        lama.isAutoMeasureEnabled = false
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_hide_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_focus_up -> binding.hideContent
                .also { it.scrollToPosition(10) }
                .also { it.smoothScrollToPosition(0) }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onPreBind(binding: FragmentHideMd2Binding) = Unit

}

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

        val maxX = revealInfo.centerX - marginEnd - measuredWidth / 2f
        val targetX = if (revealInfo.radius == 0f) 0f else -maxX
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


