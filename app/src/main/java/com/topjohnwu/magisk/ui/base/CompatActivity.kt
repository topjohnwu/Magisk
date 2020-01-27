package com.topjohnwu.magisk.ui.base

import android.app.Activity
import android.view.inputmethod.InputMethodManager
import androidx.core.content.getSystemService
import androidx.databinding.ViewDataBinding

// TODO (diareuse): Merge into BaseUIActivity after all legacy UI is migrated

abstract class CompatActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    BaseUIActivity<ViewModel, Binding>()

fun Activity.hideKeyboard() {
    val view = currentFocus ?: return
    getSystemService<InputMethodManager>()
        ?.hideSoftInputFromWindow(view.windowToken, 0)
    view.clearFocus()
}
