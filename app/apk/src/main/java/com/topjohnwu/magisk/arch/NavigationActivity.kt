package com.topjohnwu.magisk.arch

import android.view.KeyEvent
import androidx.databinding.ViewDataBinding
import androidx.navigation.NavController
import androidx.navigation.NavDirections
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.navOptions
import com.topjohnwu.magisk.utils.AccessibilityUtils

abstract class NavigationActivity<Binding : ViewDataBinding> : UIActivity<Binding>() {

    abstract val navHostId: Int

    private val navHostFragment by lazy {
        supportFragmentManager.findFragmentById(navHostId) as NavHostFragment
    }

    protected val currentFragment get() =
        navHostFragment.childFragmentManager.fragments.getOrNull(0) as? BaseFragment<*>

    val navigation: NavController get() = navHostFragment.navController

    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        return if (binded && currentFragment?.onKeyEvent(event) == true) true else super.dispatchKeyEvent(event)
    }

    override fun onBackPressed() {
        if (binded) {
            if (currentFragment?.onBackPressed() == false) {
                super.onBackPressed()
            }
        }
    }

    fun NavDirections.navigate() {
        if (AccessibilityUtils.isAnimationEnabled(contentResolver)) {
            navigation.navigate(this)
        } else {
            navigation.navigate(this, navOptions {
                anim { enter = 0; exit = 0; popEnter = 0; popExit = 0 }
            })
        }
    }
}
