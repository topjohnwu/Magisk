package com.topjohnwu.magisk.arch

import android.view.View
import androidx.core.graphics.Insets
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.lifecycle.LifecycleOwner

interface BaseUIComponent<VM : BaseViewModel>: LifecycleOwner {

    val viewRoot: View
    val viewModel: VM

    fun startObserveEvents() {
        viewModel.viewEvents.observe(this) {
            onEventDispatched(it)
        }
    }

    fun consumeSystemWindowInsets(insets: Insets): Insets? = null

    /**
     * Called for all [ViewEvent]s published by associated viewModel.
     */
    fun onEventDispatched(event: ViewEvent) {}

    fun ensureInsets() {
        ViewCompat.setOnApplyWindowInsetsListener(viewRoot) { _, insets ->
            insets.asInsets()
                .also { viewModel.insets = it }
                .let { consumeSystemWindowInsets(it) }
                ?.subtractBy(insets) ?: insets
        }
        if (ViewCompat.isAttachedToWindow(viewRoot)) {
            ViewCompat.requestApplyInsets(viewRoot)
        } else {
            viewRoot.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {
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

    private fun Insets.subtractBy(insets: WindowInsetsCompat) =
        WindowInsetsCompat.Builder(insets).setSystemWindowInsets(
            Insets.of(
                insets.systemWindowInsetLeft - left,
                insets.systemWindowInsetTop - top,
                insets.systemWindowInsetRight - right,
                insets.systemWindowInsetBottom - bottom
            )
        ).build()

}
