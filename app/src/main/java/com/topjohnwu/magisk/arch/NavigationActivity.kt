package com.topjohnwu.magisk.arch

import android.view.KeyEvent
import androidx.databinding.ViewDataBinding
import androidx.navigation.NavController
import androidx.navigation.NavDirections
import androidx.navigation.fragment.NavHostFragment

abstract class NavigationActivity<Binding : ViewDataBinding> : UIActivity<Binding>() {

    abstract val navHostId: Int

    private val navHostFragment by lazy {
        supportFragmentManager.findFragmentById(navHostId) as NavHostFragment
    }

    protected val currentFragment get() =
        navHostFragment.childFragmentManager.fragments.getOrNull(0) as? BaseFragment<*>

    val navigation: NavController get() = navHostFragment.navController

    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        return currentFocus?.let {
            currentFragment?.onKeyEvent(event).takeIf { it == true }
        } ?: super.dispatchKeyEvent(event)
    }

    override fun onBackPressed() {
        currentFocus?.let {
            if (currentFragment?.onBackPressed() == false) {
                super.onBackPressed()
            }
        }
    }

    fun NavDirections.navigate() {
        navigation.navigate(this)
    }
}
