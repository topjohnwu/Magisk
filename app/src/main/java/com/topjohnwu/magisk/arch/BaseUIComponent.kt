package com.topjohnwu.magisk.arch

import android.view.View
import androidx.lifecycle.LifecycleOwner

interface BaseUIComponent<VM : BaseViewModel> : LifecycleOwner {

    val viewRoot: View
    val viewModel: VM

    fun startObserveEvents() {
        viewModel.viewEvents.observe(this) {
            onEventDispatched(it)
        }
    }

    /**
     * Called for all [ViewEvent]s published by associated viewModel.
     */
    fun onEventDispatched(event: ViewEvent) {}
}
