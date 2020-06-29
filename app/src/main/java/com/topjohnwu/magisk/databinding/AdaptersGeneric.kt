package com.topjohnwu.magisk.databinding

import android.view.View
import android.widget.TextView
import androidx.core.text.PrecomputedTextCompat
import androidx.core.view.isGone
import androidx.core.view.isInvisible
import androidx.core.widget.TextViewCompat
import androidx.databinding.BindingAdapter
import com.topjohnwu.magisk.extensions.subscribeK
import io.reactivex.Single

@BindingAdapter("gone")
fun setGone(view: View, gone: Boolean) {
    view.isGone = gone
}

@BindingAdapter("invisible")
fun setInvisible(view: View, invisible: Boolean) {
    view.isInvisible = invisible
}

@BindingAdapter("goneUnless")
fun setGoneUnless(view: View, goneUnless: Boolean) {
    setGone(view, goneUnless.not())
}

@BindingAdapter("invisibleUnless")
fun setInvisibleUnless(view: View, invisibleUnless: Boolean) {
    setInvisible(view, invisibleUnless.not())
}

@BindingAdapter("precomputedText")
fun setPrecomputedText(tv: TextView, text: CharSequence) {
    Single.fromCallable {
        PrecomputedTextCompat.create(text, TextViewCompat.getTextMetricsParams(tv))
    }.subscribeK {
        TextViewCompat.setPrecomputedText(tv, it);
    }
}
