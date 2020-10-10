package com.topjohnwu.magisk.events

import android.view.View
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.utils.TransitiveText

class SnackbarEvent private constructor(
    private val msg: TransitiveText,
    private val length: Int,
    private val builder: Snackbar.() -> Unit
) : ViewEvent(), ActivityExecutor {

    constructor(
        @StringRes res: Int,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(TransitiveText.Res(res), length, builder)

    constructor(
        message: String,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(TransitiveText.String(message), length, builder)


    private fun snackbar(
        view: View,
        message: String,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) = Snackbar.make(view, message, length).apply(builder).show()

    override fun invoke(activity: BaseUIActivity<*, *>) {
        if (activity is BaseUIActivity<*, *>) {
            snackbar(activity.snackbarView,
                msg.getText(activity.resources).toString(),
                length, builder)
        }
    }

}
