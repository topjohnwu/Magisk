package com.topjohnwu.magisk.utils

import android.view.View
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.Toolbar
import androidx.databinding.BindingAdapter
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.state.IndeterminateState


@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(view: Toolbar, listener: View.OnClickListener) {
    view.setNavigationOnClickListener(listener)
}

@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(
    view: NavigationView,
    listener: NavigationView.OnNavigationItemSelectedListener
) {
    view.setNavigationItemSelectedListener {
        (view.parent as? DrawerLayout)?.closeDrawers()
        listener.onNavigationItemSelected(it)
    }
}

@BindingAdapter("srcCompat")
fun setImageResource(view: AppCompatImageView, @DrawableRes resId: Int) {
    view.setImageResource(resId)
}

@BindingAdapter("app:tint")
fun setTint(view: AppCompatImageView, @ColorInt tint: Int) {
    view.setColorFilter(tint)
}

@BindingAdapter("isChecked")
fun setChecked(view: AppCompatImageView, isChecked: Boolean) {
    val state = when (isChecked) {
        true -> IndeterminateState.CHECKED
        else -> IndeterminateState.UNCHECKED
    }
    setChecked(view, state)
}

@BindingAdapter("isChecked")
fun setChecked(view: AppCompatImageView, isChecked: IndeterminateState) {
    view.setImageResource(
        when (isChecked) {
            IndeterminateState.INDETERMINATE -> R.drawable.ic_indeterminate
            IndeterminateState.CHECKED -> R.drawable.ic_checked
            IndeterminateState.UNCHECKED -> R.drawable.ic_unchecked
        }
    )
}
