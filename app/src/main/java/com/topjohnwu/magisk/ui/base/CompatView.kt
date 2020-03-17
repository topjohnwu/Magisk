package com.topjohnwu.magisk.ui.base

import android.view.View
import androidx.core.graphics.Insets
import androidx.navigation.NavController

internal interface CompatView<ViewModel : BaseViewModel> {

    val viewRoot: View
    val viewModel: ViewModel
    val navigation: NavController?

    fun peekSystemWindowInsets(insets: Insets) = Unit
    fun consumeSystemWindowInsets(insets: Insets): Insets? = null

}
