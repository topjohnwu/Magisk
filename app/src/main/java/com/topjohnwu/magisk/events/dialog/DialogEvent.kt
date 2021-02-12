package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.view.MagiskDialog

abstract class DialogEvent : ViewEvent(), ActivityExecutor {

    protected lateinit var dialog: MagiskDialog

    override fun invoke(activity: BaseUIActivity<*, *>) {
        dialog = MagiskDialog(activity)
            .apply { setOwnerActivity(activity) }
            .apply(this::build).reveal()
    }

    abstract fun build(dialog: MagiskDialog)

}

typealias GenericDialogListener = () -> Unit
