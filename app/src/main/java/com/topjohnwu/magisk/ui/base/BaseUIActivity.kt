package com.topjohnwu.magisk.ui.base

import android.os.Bundle
import androidx.appcompat.app.AppCompatDelegate
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.model.events.EventHandler

abstract class BaseUIActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
        BaseActivity(), EventHandler {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    abstract val viewModel: ViewModel
    protected open val themeRes: Int = R.style.MagiskTheme

    open val snackbarView get() = binding.root

    init {
        val theme = Config.darkThemeExtended
        AppCompatDelegate.setDefaultNightMode(theme)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(themeRes)
        super.onCreate(savedInstanceState)

        viewModel.viewEvents.observe(this, viewEventObserver)

        binding = DataBindingUtil.setContentView<Binding>(this, layoutRes).apply {
            setVariable(BR.viewModel, viewModel)
            lifecycleOwner = this@BaseUIActivity
        }
    }
}
