package com.topjohnwu.magisk.ui.compat

import android.view.View
import androidx.core.graphics.Insets

internal interface CompatView<ViewModel : CompatViewModel> {

    val viewRoot: View
    val viewModel: ViewModel
    val navigation: CompatNavigationDelegate<*>?

    fun peekSystemWindowInsets(insets: Insets) = Unit
    fun consumeSystemWindowInsets(insets: Insets): Insets? = null

}
