package com.topjohnwu.magisk.arch

import android.content.Intent
import android.os.Bundle
import android.view.KeyEvent
import android.view.View
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.res.use
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import androidx.lifecycle.MutableLiveData
import androidx.navigation.NavController
import androidx.navigation.NavDirections
import androidx.navigation.fragment.NavHostFragment
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.ui.theme.Theme

abstract class BaseUIActivity<VM : BaseViewModel, Binding : ViewDataBinding> :
    BaseActivity(), BaseUIComponent<VM> {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected open val themeRes: Int = Theme.selected.themeRes

    private val navHostFragment by lazy {
        supportFragmentManager.findFragmentById(navHost) as? NavHostFragment
    }
    private val topFragment get() = navHostFragment?.childFragmentManager?.fragments?.getOrNull(0)
    protected val currentFragment get() = topFragment as? BaseUIFragment<*, *>

    override val viewRoot: View get() = binding.root
    open val navigation: NavController? get() = navHostFragment?.navController

    open val navHost: Int = 0
    open val snackbarView get() = binding.root

    init {
        val theme = Config.darkTheme
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
        startObserveEvents()

        binding = DataBindingUtil.setContentView<Binding>(this, layoutRes).also {
            it.setVariable(BR.viewModel, viewModel)
            it.lifecycleOwner = this
        }

        ensureInsets()

        directionsDispatcher.observe(this) {
            it?.navigate()
            // we don't want the directions to be re-dispatched, so we preemptively set them to null
            if (it != null) {
                directionsDispatcher.value = null
            }
        }
    }

    override fun onResume() {
        super.onResume()
        viewModel.requestRefresh()
    }

    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        return currentFragment?.onKeyEvent(event) == true || super.dispatchKeyEvent(event)
    }

    override fun onEventDispatched(event: ViewEvent) = when(event) {
        is ContextExecutor -> event(this)
        is ActivityExecutor -> event(this)
        else -> Unit
    }

    override fun onBackPressed() {
        if (navigation == null || currentFragment?.onBackPressed()?.not() == true) {
            super.onBackPressed()
        }
    }

    fun NavDirections.navigate() {
        navigation?.navigate(this)
    }

    companion object {

        private val directionsDispatcher = MutableLiveData<NavDirections?>()

        fun postDirections(navDirections: NavDirections) =
            directionsDispatcher.postValue(navDirections)

    }

}
