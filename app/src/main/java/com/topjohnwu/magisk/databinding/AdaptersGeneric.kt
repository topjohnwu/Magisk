package com.topjohnwu.magisk.databinding

import android.view.View
import androidx.core.view.isGone
import androidx.core.view.isInvisible
import androidx.databinding.BindingAdapter

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
