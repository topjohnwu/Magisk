
package com.topjohnwu.magisk.ui.module

import android.widget.Button
import androidx.databinding.BindingAdapter
import com.google.android.material.R
import com.google.android.material.color.MaterialColors
import android.content.res.ColorStateList
import com.google.android.material.button.MaterialButton

@BindingAdapter("updateButtonAppearance")
fun setUpdateButtonAppearance(button: MaterialButton, isReady: Boolean) {
    button.alpha = if (isReady) 1.0f else 0.38f
    val colorAttr = if (isReady) R.attr.colorPrimary else R.attr.colorOnSurface
    val color = MaterialColors.getColor(button, colorAttr)
    button.setTextColor(color)
    button.iconTint = ColorStateList.valueOf(color)
} 