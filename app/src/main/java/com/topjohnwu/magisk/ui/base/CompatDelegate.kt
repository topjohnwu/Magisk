package com.topjohnwu.magisk.ui.base

import android.view.View
import androidx.core.graphics.Insets
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ContextExecutor
import com.topjohnwu.magisk.model.events.FragmentExecutor
import com.topjohnwu.magisk.model.events.ViewEvent
import timber.log.Timber


class CompatDelegate internal constructor(
    private val view: CompatView<*>
) {

    fun onCreate() {
        ensureInsets()
    }

    fun onResume() {
        view.viewModel.requestRefresh()
    }

    fun onEventExecute(event: ViewEvent, activity: BaseUIActivity<*, *>) {
        (event as? ContextExecutor)?.invoke(activity)
        (event as? ActivityExecutor)?.invoke(activity)
        (event as? FragmentExecutor)?.let {
            Timber.e("Cannot run ${FragmentExecutor::class.java.simpleName} in Activity. Consider adding ${ContextExecutor::class.java.simpleName} as fallback.")
        }
    }

    fun onEventExecute(event: ViewEvent, fragment: Fragment) {
        (event as? ContextExecutor)?.invoke(fragment.requireContext())
        (event as? FragmentExecutor)?.invoke(fragment)
        (event as? ActivityExecutor)?.invoke(fragment.requireActivity() as BaseUIActivity<*, *>)
    }

    private fun ensureInsets() {
        ViewCompat.setOnApplyWindowInsetsListener(view.viewRoot) { _, insets ->
            insets.asInsets()
                .also { view.peekSystemWindowInsets(it) }
                .let { view.consumeSystemWindowInsets(it) }
                ?.also { view.viewModel.insets.value = it }
                ?.subtractBy(insets) ?: insets
        }
        if (ViewCompat.isAttachedToWindow(view.viewRoot)) {
            ViewCompat.requestApplyInsets(view.viewRoot)
        } else {
            view.viewRoot.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {
                override fun onViewDetachedFromWindow(v: View) = Unit
                override fun onViewAttachedToWindow(v: View) {
                    ViewCompat.requestApplyInsets(v)
                }
            })
        }
    }

    private fun WindowInsetsCompat.asInsets() = Insets.of(
        systemWindowInsetLeft,
        systemWindowInsetTop,
        systemWindowInsetRight,
        systemWindowInsetBottom
    )

    private fun Insets.subtractBy(insets: WindowInsetsCompat) = insets.replaceSystemWindowInsets(
        insets.systemWindowInsetLeft - left,
        insets.systemWindowInsetTop - top,
        insets.systemWindowInsetRight - right,
        insets.systemWindowInsetBottom - bottom
    )

}
