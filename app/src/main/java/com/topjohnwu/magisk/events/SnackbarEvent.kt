package com.topjohnwu.magisk.events

import android.view.View
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.utils.TransitiveText
import com.topjohnwu.magisk.utils.asTransitive

class SnackbarEvent constructor(
    private val msg: TransitiveText,
    private val length: Int = Snackbar.LENGTH_SHORT,
    private val builder: Snackbar.() -> Unit = {}
) : ViewEvent(), ActivityExecutor {

    constructor(
        @StringRes res: Int,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(res.asTransitive(), length, builder)

    constructor(
        msg: String,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(msg.asTransitive(), length, builder)


    private fun snackbar(
        view: View,
        message: String,
        length: Int,
        builder: Snackbar.() -> Unit
    ) = Snackbar.make(view, message, length).apply(builder).show()

    override fun invoke(activity: BaseUIActivity<*, *>) {
        snackbar(activity.snackbarView,
            msg.getText(activity.resources).toString(),
            length, builder)
    }
}
