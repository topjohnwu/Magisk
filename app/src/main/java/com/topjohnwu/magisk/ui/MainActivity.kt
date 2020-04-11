package com.topjohnwu.magisk.ui

import android.os.Bundle
import android.view.MenuItem
import android.view.View
import android.view.ViewTreeObserver
import android.view.WindowManager
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.graphics.Insets
import androidx.core.view.forEach
import androidx.core.view.setPadding
import androidx.core.view.updateLayoutParams
import androidx.navigation.NavDirections
import com.google.android.material.card.MaterialCardView
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.ui.base.BaseUIActivity
import com.topjohnwu.magisk.ui.home.HomeFragmentDirections
import com.topjohnwu.magisk.utils.HideBottomViewOnScrollBehavior
import com.topjohnwu.magisk.utils.HideTopViewOnScrollBehavior
import com.topjohnwu.magisk.utils.HideableBehavior
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel

open class MainActivity : BaseUIActivity<MainViewModel, ActivityMainMd2Binding>() {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()
    override val navHost: Int = R.id.main_nav_host

    //This temporarily fixes unwanted feature of BottomNavigationView - where the view applies
    //padding on itself given insets are not consumed beforehand. Unfortunately the listener
    //implementation doesn't favor us against the design library, so on re-create it's often given
    //upper hand.
    private val navObserver = ViewTreeObserver.OnGlobalLayoutListener {
        binding.mainNavigation.setPadding(0)
    }

    protected var isRoot = true
        private set

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Info.env.isUnsupported) {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_magisk_title)
                .applyMessage(R.string.unsupport_magisk_msg, Const.Version.MIN_VERSION)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) { titleRes = android.R.string.ok }
                .cancellable(true)
                .reveal()
        }

        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE)

        navigation?.addOnDestinationChangedListener { controller, destination, arguments ->
            isRoot = when (destination.id) {
                R.id.homeFragment,
                R.id.modulesFragment,
                R.id.superuserFragment,
                R.id.logFragment -> true
                else -> false
            }

            setDisplayHomeAsUpEnabled(!isRoot)
            requestNavigationHidden(!isRoot)

            binding.mainNavigation.menu.forEach {
                if (it.itemId == destination.id) {
                    it.isChecked = true
                }
            }
        }

        setSupportActionBar(binding.mainToolbar)

        binding.mainToolbarWrapper.updateLayoutParams<CoordinatorLayout.LayoutParams> {
            behavior = HideTopViewOnScrollBehavior<MaterialCardView>()
        }
        binding.mainBottomBar.updateLayoutParams<CoordinatorLayout.LayoutParams> {
            behavior = HideBottomViewOnScrollBehavior<MaterialCardView>()
        }
        binding.mainNavigation.setOnNavigationItemSelectedListener {
            when (it.itemId) {
                R.id.homeFragment -> MainDirections.actionHomeFragment()
                R.id.modulesFragment -> MainDirections.actionModuleFragment()
                R.id.superuserFragment -> MainDirections.actionSuperuserFragment()
                R.id.logFragment -> MainDirections.actionLogFragment()
                else -> throw NotImplementedError("Id ${it.itemId} is not defined as selectable")
            }.navigate()
            true
        }
        binding.mainNavigation.setOnNavigationItemReselectedListener {
            (currentFragment as? ReselectionTarget)?.onReselected()
        }

        binding.mainNavigation.viewTreeObserver.addOnGlobalLayoutListener(navObserver)

        when {
            intent.hasExtra(Const.Key.OPEN_SECTION) ->
                getScreen(intent.getStringExtra(Const.Key.OPEN_SECTION))?.navigate()
            intent.getBooleanExtra(Const.Key.OPEN_SETTINGS, false) ->
                HomeFragmentDirections.actionHomeFragmentToSettingsFragment().navigate()
        }


        if (savedInstanceState != null) {
            if (!isRoot) {
                requestNavigationHidden()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        binding.mainNavigation.menu.apply {
            val isRoot = Shell.rootAccess()
            findItem(R.id.modulesFragment)?.isEnabled = isRoot
            findItem(R.id.superuserFragment)?.isEnabled = isRoot
        }
    }

    override fun onDestroy() {
        binding.mainNavigation.viewTreeObserver.removeOnGlobalLayoutListener(navObserver)
        super.onDestroy()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            android.R.id.home -> onBackPressed()
            else -> return super.onOptionsItemSelected(item)
        }
        return true
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    fun setDisplayHomeAsUpEnabled(isEnabled: Boolean) {
        binding.mainToolbar.startAnimations()
        when {
            isEnabled -> binding.mainToolbar.setNavigationIcon(R.drawable.ic_back_md2)
            else -> binding.mainToolbar.navigationIcon = null
        }
    }

    @Suppress("UNCHECKED_CAST")
    internal fun requestNavigationHidden(hide: Boolean = true) {
        val topView = binding.mainToolbarWrapper
        val bottomView = binding.mainBottomBar

        val topParams = topView.layoutParams as? CoordinatorLayout.LayoutParams
        val bottomParams = bottomView.layoutParams as? CoordinatorLayout.LayoutParams

        val topBehavior = topParams?.behavior as? HideableBehavior<View>
        val bottomBehavior = bottomParams?.behavior as? HideableBehavior<View>

        topBehavior?.setHidden(topView, hide = false, lockState = false)
        bottomBehavior?.setHidden(bottomView, hide, hide)
    }

    fun invalidateToolbar() {
        //binding.mainToolbar.startAnimations()
        binding.mainToolbar.invalidate()
    }

    private fun getScreen(name: String?): NavDirections? {
        return when (name) {
            "superuser" -> HomeFragmentDirections.actionSuperuserFragment()
            "magiskhide" -> HomeFragmentDirections.actionHideFragment()
            "modules" -> HomeFragmentDirections.actionModuleFragment()
            null -> null
            else -> TODO("Implement screen shortcut \"$name\"")
        }
    }

}
