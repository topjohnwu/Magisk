package com.topjohnwu.magisk.redesign.compat

import android.graphics.Insets
import android.view.View

internal interface CompatView<ViewModel : CompatViewModel> {

    val viewRoot: View
    val viewModel: ViewModel
    val navigation: CompatNavigationDelegate<*>?

    fun peekSystemWindowInsets(insets: Insets) = Unit
    fun consumeSystemWindowInsets(insets: Insets) = Insets.NONE

}