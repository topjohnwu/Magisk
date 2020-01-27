package com.topjohnwu.magisk.ui.base

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.graphics.Insets
import androidx.databinding.DataBindingUtil
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.theme.Theme
import kotlin.reflect.KClass

abstract class BaseUIActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    BaseActivity(), CompatView<ViewModel>, EventHandler {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected open val themeRes: Int = Theme.selected.themeRes

    override val viewRoot: View get() = binding.root
    override val navigation by lazy { CompatNavigationDelegate(this) as CompatNavigationDelegate? }

    private val delegate by lazy { CompatDelegate(this) }

    open val navHost: Int = 0
    open val snackbarView get() = binding.root
    open val baseFragments = listOf<KClass<out Fragment>>()

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

        binding.addOnRebindCallback(object : OnRebindCallback<Binding>() {
            override fun onPreBind(binding: Binding): Boolean {
                (binding.root as? ViewGroup)?.startAnimations()
                return super.onPreBind(binding)
            }
        })

        delegate.onCreate()
        navigation?.onCreate(savedInstanceState)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        navigation?.onActivityResult(requestCode, resultCode, data)
    }

    override fun onResume() {
        super.onResume()
        delegate.onResume()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        navigation?.onSaveInstanceState(outState)
    }

    override fun onEventDispatched(event: ViewEvent) {
        delegate.onEventExecute(event, this)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(this), event.length, event.f)
        }
    }

    override fun onBackPressed() {
        if (navigation == null || navigation?.onBackPressed()?.not() == true) {
            super.onBackPressed()
        }
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    protected fun ViewEvent.dispatchOnSelf() = onEventDispatched(this)
}
