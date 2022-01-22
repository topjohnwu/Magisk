package com.topjohnwu.magisk.events

import android.view.View
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.utils.TextHolder
import com.topjohnwu.magisk.utils.asText

class SnackbarEvent constructor(
    private val msg: TextHolder,
    private val length: Int = Snackbar.LENGTH_SHORT,
    private val builder: Snackbar.() -> Unit = {}
) : ViewEvent(), ActivityExecutor {

    constructor(
        @StringRes res: Int,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(res.asText(), length, builder)

    constructor(
        msg: String,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(msg.asText(), length, builder)


    private fun snackbar(
        view: View,
        anchor: View?,
        message: String,
        length: Int,
        builder: Snackbar.() -> Unit
    ) = Snackbar.make(view, message, length).setAnchorView(anchor).apply(builder).show()

    override fun invoke(activity: UIActivity<*>) {
        snackbar(activity.snackbarView, activity.snackbarAnchorView,
            msg.getText(activity.resources).toString(),
            length, builder)
    }
}
