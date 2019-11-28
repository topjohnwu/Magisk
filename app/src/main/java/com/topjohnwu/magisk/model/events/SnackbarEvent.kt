package com.topjohnwu.magisk.model.events

import android.content.Context
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar

class SnackbarEvent private constructor(
    @StringRes private val messageRes: Int,
    private val messageString: String?,
    val length: Int,
    val f: Snackbar.() -> Unit
) : ViewEvent() {

    constructor(
        @StringRes messageRes: Int,
        length: Int = Snackbar.LENGTH_SHORT,
        f: Snackbar.() -> Unit = {}
    ) : this(messageRes, null, length, f)

    constructor(
        message: String,
        length: Int = Snackbar.LENGTH_SHORT,
        f: Snackbar.() -> Unit = {}
    ) : this(-1, message, length, f)

    fun message(context: Context): String = messageString ?: context.getString(messageRes)
}
