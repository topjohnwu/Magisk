package com.topjohnwu.magisk.arch

import androidx.lifecycle.LifecycleOwner

interface ViewModelHolder : LifecycleOwner {

    val viewModel: BaseViewModel

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
