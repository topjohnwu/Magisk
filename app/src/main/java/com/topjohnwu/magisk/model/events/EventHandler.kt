package com.topjohnwu.magisk.model.events

internal interface EventHandler {

    /**
     * Called for all [ViewEvent]s published by associated viewModel.
     * For [SimpleViewEvent]s, both this and [onSimpleEventDispatched]
     * methods are called - you can choose the way how you handle them.
     */
    fun onEventDispatched(event: ViewEvent) {}

    /**
     * Called for all [SimpleViewEvent]s published by associated viewModel.
     * Both this and [onEventDispatched] methods are called - you can choose
     * the way how you handle them.
     */
    fun onSimpleEventDispatched(event: Int) {}

    val viewEventObserver get() = ViewEventObserver {
        onEventDispatched(it)
        if (it is SimpleViewEvent) {
            onSimpleEventDispatched(it.event)
        }
    }
}
