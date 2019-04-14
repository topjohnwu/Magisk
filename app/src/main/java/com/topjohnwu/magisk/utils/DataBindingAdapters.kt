package com.topjohnwu.magisk.utils

import android.view.View
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.Toolbar
import androidx.databinding.BindingAdapter


@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(view: Toolbar, listener: View.OnClickListener) {
    view.setNavigationOnClickListener(listener)
}

@BindingAdapter("srcCompat")
fun setImageResource(view: AppCompatImageView, @DrawableRes resId: Int) {
    view.setImageResource(resId)
}

@BindingAdapter("app:tint")
fun setTint(view: AppCompatImageView, @ColorInt tint: Int) {
    view.setColorFilter(tint)
}
