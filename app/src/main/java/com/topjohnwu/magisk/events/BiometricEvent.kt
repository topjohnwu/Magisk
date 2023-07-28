package com.topjohnwu.magisk.events

import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.core.di.ServiceLocator

class BiometricEvent(
    builder: Builder.() -> Unit
) : ViewEvent(), ActivityExecutor {

    private var listenerOnFailure: () -> Unit = {}
    private var listenerOnSuccess: () -> Unit = {}

    init {
        builder(Builder())
    }

    override fun invoke(activity: UIActivity<*>) {
        ServiceLocator.biometrics.authenticate(
            activity,
            onError = listenerOnFailure,
            onSuccess = listenerOnSuccess
        )
    }

    inner class Builder internal constructor() {

        fun onFailure(listener: () -> Unit) {
            listenerOnFailure = listener
        }

        fun onSuccess(listener: () -> Unit) {
            listenerOnSuccess = listener
        }
    }

}
