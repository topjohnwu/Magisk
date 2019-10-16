package com.topjohnwu.magisk.redesign.compat

import android.graphics.Insets
import androidx.appcompat.app.AppCompatActivity
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

    fun onEventExecute(event: ViewEvent, activity: AppCompatActivity) {
        (event as? ContextExecutor)?.invoke(activity)
        (event as? ActivityExecutor)?.invoke(activity)
        (event as? FragmentExecutor)?.let {
            Timber.e("Cannot run ${FragmentExecutor::class.java.simpleName} in Activity. Consider adding ${ContextExecutor::class.java.simpleName} as fallback.")
        }
    }

    fun onEventExecute(event: ViewEvent, fragment: Fragment) {
        (event as? ContextExecutor)?.invoke(fragment.requireContext())
        (event as? FragmentExecutor)?.invoke(fragment)
        (event as? ActivityExecutor)?.invoke(fragment.requireActivity() as AppCompatActivity)
    }

    private fun ensureInsets() {
        ViewCompat.setOnApplyWindowInsetsListener(view.viewRoot) { _, insets ->
            insets.asInsets()
                .also { view.peekSystemWindowInsets(it) }
                .let { view.consumeSystemWindowInsets(it) }
                .also { if (it != Insets.NONE) view.viewModel.insets.value = it }
                .subtractBy(insets)
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