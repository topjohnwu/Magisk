package com.topjohnwu.magisk.viewmodel

abstract class StatefulViewModel<State : Enum<*>>(
    val defaultState: State
) : ObservableViewModel() {

    var state: State = defaultState
        set(value) {
            field = value
            notifyStateChanged()
        }

    open fun notifyStateChanged() = Unit

}