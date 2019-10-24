package com.topjohnwu.magisk.redesign.safetynet

import android.os.Build
import android.view.View
import android.view.ViewAnimationUtils
import androidx.core.animation.doOnEnd
import androidx.core.view.isInvisible
import androidx.core.view.isVisible
import androidx.databinding.BindingAdapter
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSafetynetMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.math.hypot
import kotlin.math.roundToInt

class SafetynetFragment : CompatFragment<SafetynetViewModel, FragmentSafetynetMd2Binding>() {

    override val layoutRes = R.layout.fragment_safetynet_md2
    override val viewModel by viewModel<SafetynetViewModel>()

    override fun onStart() {
        super.onStart()
        activity.setTitle(R.string.safetyNet)
    }

}

@BindingAdapter("revealSafetyNet")
fun View.revealOnCenter(expand: Boolean) {
    val anim = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
        isInvisible = expand
        return
    } else {
        val x = measuredWidth.toDouble()
        val y = measuredHeight.toDouble()
        val maxRadius = hypot(x, y).toFloat()
        val start = if (expand) 0f else maxRadius
        val end = if (expand) maxRadius else 0f

        ViewAnimationUtils.createCircularReveal(
            this,
            (x / 2).roundToInt(),
            (y / 2).roundToInt(),
            start,
            end
        ).apply {
            interpolator = FastOutSlowInInterpolator()
            doOnEnd { if (!expand) isVisible = false }
        }
    }

    post {
        isVisible = true
        anim.start()
    }
}