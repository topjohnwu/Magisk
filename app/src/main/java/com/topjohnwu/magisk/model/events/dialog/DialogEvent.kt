package com.topjohnwu.magisk.model.events.dialog

import android.content.Context
import com.topjohnwu.magisk.model.events.ContextExecutor
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.view.MagiskDialog

abstract class DialogEvent : ViewEvent(), ContextExecutor {

    protected lateinit var dialog: MagiskDialog

    override fun invoke(context: Context) {
        dialog = MagiskDialog(context).apply(this::build).reveal()
    }

    abstract fun build(dialog: MagiskDialog)

}

typealias GenericDialogListener = () -> Unit