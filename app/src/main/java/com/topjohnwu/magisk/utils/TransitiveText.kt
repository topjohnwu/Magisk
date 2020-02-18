package com.topjohnwu.magisk.utils

import android.content.res.Resources
import android.widget.TextView
import androidx.databinding.BindingAdapter
import androidx.databinding.InverseBindingAdapter
import com.topjohnwu.magisk.extensions.get

sealed class TransitiveText {

    abstract val isEmpty: Boolean
    abstract fun getText(resources: Resources): CharSequence

    // ---

    class String(
        private val value: CharSequence
    ) : TransitiveText() {

        override val isEmpty = value.isEmpty()
        override fun getText(resources: Resources) = value

    }

    class Res(
        private val value: Int,
        private vararg val params: Any
    ) : TransitiveText() {

        override val isEmpty = value == 0
        override fun getText(resources: Resources) =
            resources.getString(value, *params)

    }

    // ---

    companion object {
        val EMPTY = String("")
    }

}


fun Int.asTransitive(vararg params: Any) = TransitiveText.Res(this, *params)
fun CharSequence.asTransitive() = TransitiveText.String(this)


@BindingAdapter("android:text")
fun TextView.setText(text: TransitiveText) {
    this.text = text.getText(get())
}

@InverseBindingAdapter(attribute = "android:text", event = "android:textAttrChanged")
fun TextView.getTransitiveText() = text.asTransitive()
