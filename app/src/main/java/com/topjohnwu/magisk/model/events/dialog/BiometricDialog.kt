package com.topjohnwu.magisk.model.events.dialog

import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.utils.BiometricHelper

class BiometricDialog(
    builder: Builder.() -> Unit
) : ViewEvent(), ActivityExecutor {

    private var listenerOnFailure: GenericDialogListener = {}
    private var listenerOnSuccess: GenericDialogListener = {}

    init {
        builder(Builder())
    }

    override fun invoke(activity: AppCompatActivity) {
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