package com.topjohnwu.magisk.utils

import android.view.View
import androidx.appcompat.widget.Toolbar
import androidx.databinding.BindingAdapter


@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(view: Toolbar, listener: View.OnClickListener) {
    view.setNavigationOnClickListener(listener)
}
