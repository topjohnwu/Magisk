package com.topjohnwu.magisk.ui.base

import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment

abstract class CompatFragment<ViewModel : BaseViewModel, Binding : ViewDataBinding>
    : BaseUIFragment<ViewModel, Binding>(), CompatView<ViewModel>

fun Fragment.hideKeyboard() {
    activity?.hideKeyboard()
}
