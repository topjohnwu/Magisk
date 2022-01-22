package com.topjohnwu.magisk.events.dialog

import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.view.MagiskDialog

abstract class DialogEvent : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: UIActivity<*>) {
        MagiskDialog(activity)
            .apply { setOwnerActivity(activity) }
            .apply(this::build).show()
    }

    abstract fun build(dialog: MagiskDialog)

}

typealias GenericDialogListener = () -> Unit
