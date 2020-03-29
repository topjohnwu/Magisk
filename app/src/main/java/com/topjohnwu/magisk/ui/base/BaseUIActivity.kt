package com.topjohnwu.magisk.ui.base

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.res.use
import androidx.core.graphics.Insets
import androidx.databinding.DataBindingUtil
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.Observer
import androidx.navigation.NavDirections
import androidx.navigation.findNavController
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.theme.Theme

abstract class BaseUIActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    BaseActivity(), CompatView<ViewModel>, EventHandler {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected open val themeRes: Int = Theme.selected.themeRes

    private val navHostFragment get() = supportFragmentManager.findFragmentById(navHost)
    private val topFragment get() = navHostFragment?.childFragmentManager?.fragments?.getOrNull(0)
    protected val currentFragment get() = topFragment as? BaseUIFragment<*, *>

    override val viewRoot: View get() = binding.root
    override val navigation by lazy {
        kotlin.runCatching { findNavController(navHost) }.getOrNull()
    }

    private val delegate by lazy { CompatDelegate(this) }

    open val navHost: Int = 0
    open val snackbarView get() = binding.root

    init {
        val theme = Config.darkThemeExtended
        AppCompatDelegate.setDefaultNightMode(theme)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        currentFragment?.onActivityResult(requestCode, resultCode, data)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(themeRes)

        // We need to set the window background explicitly since for whatever reason it's not
        // propagated upstream
        obtainStyledAttributes(intArrayOf(android.R.attr.windowBackground))
            .use { it.getDrawable(0) }
            .also { window.setBackgroundDrawable(it) }

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

        directionsDispatcher.observe(this, Observer {
            it?.navigate()
            // we don't want the directions to be re-dispatched, so we preemptively set them to null
            if (it != null) {
                directionsDispatcher.value = null
            }
        })
    }

    override fun onResume() {
        super.onResume()
        delegate.onResume()
    }

    override fun onEventDispatched(event: ViewEvent) {
        delegate.onEventExecute(event, this)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(this), event.length, event.f)
        }
    }

    override fun onBackPressed() {
        if (navigation == null || currentFragment?.onBackPressed()?.not() == true) {
            super.onBackPressed()
        }
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    protected fun ViewEvent.dispatchOnSelf() = onEventDispatched(this)

    fun NavDirections.navigate() {
        navigation?.navigate(this)
    }

    companion object {

        private val directionsDispatcher = MutableLiveData<NavDirections?>()

        fun postDirections(navDirections: NavDirections) =
            directionsDispatcher.postValue(navDirections)

    }

}
