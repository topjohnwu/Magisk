package com.topjohnwu.magisk.redesign.compat

import android.graphics.Insets
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat

class CompatDelegate internal constructor(
    private val view: CompatView<*>
) {

    fun onResume() {
        view.viewModel.requestRefresh()
    }

    fun ensureInsets() {
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