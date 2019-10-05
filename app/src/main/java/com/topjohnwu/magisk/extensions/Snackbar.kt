package com.topjohnwu.magisk.extensions

import android.content.Context
import android.content.res.ColorStateList
import android.view.View
import android.widget.TextView
import androidx.annotation.ColorInt
import androidx.annotation.ColorRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.view.ViewCompat
import androidx.fragment.app.Fragment
import com.google.android.material.snackbar.Snackbar

fun AppCompatActivity.snackbar(
    view: View,
    @StringRes messageRes: Int,
    length: Int = Snackbar.LENGTH_SHORT,
    f: Snackbar.() -> Unit = {}
) {
    snackbar(view, getString(messageRes), length, f)
}

fun AppCompatActivity.snackbar(
    view: View,
    message: String,
    length: Int = Snackbar.LENGTH_SHORT,
    f: Snackbar.() -> Unit = {}
) = Snackbar.make(view, message, length)
    .apply(f)
    .show()

fun Fragment.snackbar(
    view: View,
    @StringRes messageRes: Int,
    length: Int = Snackbar.LENGTH_SHORT,
    f: Snackbar.() -> Unit = {}
) {
    snackbar(view, getString(messageRes), length, f)
}

fun Fragment.snackbar(
    view: View,
    message: String,
    length: Int = Snackbar.LENGTH_SHORT,
    f: Snackbar.() -> Unit = {}
) = Snackbar.make(view, message, length)
    .apply(f)
    .show()

fun Snackbar.action(init: KSnackbar.() -> Unit) = apply {
    val config = KSnackbar().apply(init)

    setAction(config.title(context), config.onClickListener)

    when {
        config.hasValidColor -> setActionTextColor(config.color(context) ?: return@apply)
        config.hasValidColorStateList -> setActionTextColor(config.colorStateList(context) ?: return@apply)
    }
}

class KSnackbar {
    var colorRes: Int = -1
    var colorStateListRes: Int = -1

    var title: CharSequence = ""
    var titleRes: Int = -1

    internal var onClickListener: (View) -> Unit = {}
    internal val hasValidColor get() = colorRes != -1
    internal val hasValidColorStateList get() = colorStateListRes != -1

    fun onClicked(listener: (View) -> Unit) {
        onClickListener = listener
    }

    internal fun title(context: Context) = if (title.isBlank()) context.getString(titleRes) else title
    internal fun colorStateList(context: Context) = context.colorStateListCompat(colorStateListRes)
    internal fun color(context: Context) = context.colorCompat(colorRes)
}

@Deprecated("Kotlin DSL version is preferred", ReplaceWith("action {}"))
fun Snackbar.action(
    @StringRes actionRes: Int,
    @ColorRes colorRes: Int? = null,
    listener: (View) -> Unit
) {
    view.resources.getString(actionRes)
    colorRes?.let { ContextCompat.getColor(view.context, colorRes) }
    action {}
}

@Deprecated("Kotlin DSL version is preferred", ReplaceWith("action {}"))
fun Snackbar.action(action: String, @ColorInt color: Int? = null, listener: (View) -> Unit) {
    setAction(action, listener)
    color?.let { setActionTextColor(color) }
}

fun Snackbar.textColorRes(@ColorRes colorRes: Int) {
    textColor(context.colorCompat(colorRes) ?: return)
}

fun Snackbar.textColor(@ColorInt color: Int) {
    val tv = view.findViewById<TextView>(com.google.android.material.R.id.snackbar_text)
    tv.setTextColor(color)
}

fun Snackbar.backgroundColorRes(@ColorRes colorRes: Int) {
    backgroundColor(context.colorCompat(colorRes) ?: return)
}

fun Snackbar.backgroundColor(@ColorInt color: Int) {
    ViewCompat.setBackgroundTintList(
        view,
        ColorStateList.valueOf(color)
    )
}

fun Snackbar.alert() {
    textColor(0xF44336)
}

fun Snackbar.success() {
    textColor(0x4CAF50)
}