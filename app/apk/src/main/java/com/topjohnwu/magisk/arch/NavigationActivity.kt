package com.topjohnwu.magisk.arch

import android.content.ContentResolver
import android.os.Bundle
import android.view.KeyEvent
import androidx.activity.OnBackPressedCallback
import androidx.databinding.ViewDataBinding
import androidx.navigation.NavController
import androidx.navigation.NavDirections
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.navOptions
import com.topjohnwu.magisk.utils.AccessibilityUtils

abstract class NavigationActivity<Binding : ViewDataBinding> : UIActivity<Binding>() {

    abstract val navHostId: Int

    private val navigationBackCallback = object : OnBackPressedCallback(true) {
        override fun handleOnBackPressed() {
            if (!binded) {
                isEnabled = false
                onBackPressedDispatcher.onBackPressed()
                isEnabled = true
                return
            }
            if (currentFragment?.onBackPressed() == false) {
                isEnabled = false
                onBackPressedDispatcher.onBackPressed()
                isEnabled = true
            }
        }
    }

    private val navHostFragment by lazy {
        supportFragmentManager.findFragmentById(navHostId) as NavHostFragment
    }

    protected val currentFragment get() =
        navHostFragment.childFragmentManager.fragments.getOrNull(0) as? BaseFragment<*>

    val navigation: NavController get() = navHostFragment.navController

    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        return if (binded && currentFragment?.onKeyEvent(event) == true) true else super.dispatchKeyEvent(event)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        onBackPressedDispatcher.addCallback(this, navigationBackCallback)
    }

    companion object {
        fun navigate(directions: NavDirections, navigation: NavController, cr: ContentResolver) {
            if (AccessibilityUtils.isAnimationEnabled(cr)) {
                navigation.navigate(directions)
            } else {
                navigation.navigate(directions, navOptions {})
            }
        }
    }

    fun NavDirections.navigate() {
        navigate(this, navigation, contentResolver)
    }
}
