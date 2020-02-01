package com.topjohnwu.magisk.model.events.dialog

import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ViewEvent

class BiometricDialog(
    builder: Builder.() -> Unit
) : ViewEvent(), ActivityExecutor {

    private var listenerOnFailure: GenericDialogListener = {}
    private var listenerOnSuccess: GenericDialogListener = {}

    init {
        builder(Builder())
    }

    override fun invoke(activity: BaseActivity) {
        BiometricHelper.authenticate(
            activity,
            onError = listenerOnFailure,
            onSuccess = listenerOnSuccess
        )
    }

    inner class Builder internal constructor() {

        fun onFailure(listener: GenericDialogListener) {
            listenerOnFailure = listener
        }

        fun onSuccess(listener: GenericDialogListener) {
            listenerOnSuccess = listener
        }
    }

}
