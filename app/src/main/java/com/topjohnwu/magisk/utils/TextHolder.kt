package com.topjohnwu.magisk.utils

import android.content.res.Resources
import android.widget.TextView
import androidx.databinding.BindingAdapter

abstract class TextHolder {

    open val isEmpty: Boolean get() = false
    abstract fun getText(resources: Resources): CharSequence

    // ---

    class String(
        private val value: CharSequence
    ) : TextHolder() {

        override val isEmpty get() = value.isEmpty()
        override fun getText(resources: Resources) = value

    }

    class Resource(
        private val value: Int,
        private vararg val params: Any
    ) : TextHolder() {

        override val isEmpty get() = value == 0
        override fun getText(resources: Resources) = resources.getString(value, *params)

    }

    // ---

    companion object {
        val EMPTY = String("")
    }
}

fun Int.asText(vararg params: Any): TextHolder = TextHolder.Resource(this, *params)
fun CharSequence.asText(): TextHolder = TextHolder.String(this)


@BindingAdapter("android:text")
fun TextView.setText(text: TextHolder) {
    this.text = text.getText(context.resources)
}
