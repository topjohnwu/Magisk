package com.topjohnwu.magisk.redesign

import android.graphics.Insets
import android.os.Bundle
import android.view.MenuItem
import android.view.ViewGroup
import android.view.ViewTreeObserver
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.view.isVisible
import androidx.core.view.setPadding
import androidx.core.view.updateLayoutParams
import androidx.fragment.app.Fragment
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import com.google.android.material.card.MaterialCardView
import com.ncapdevi.fragnav.FragNavController
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatActivity
import com.topjohnwu.magisk.redesign.compat.CompatNavigationDelegate
import com.topjohnwu.magisk.redesign.home.HomeFragment
import com.topjohnwu.magisk.redesign.log.LogFragment
import com.topjohnwu.magisk.redesign.module.ModuleFragment
import com.topjohnwu.magisk.redesign.settings.SettingsFragment
import com.topjohnwu.magisk.redesign.superuser.SuperuserFragment
import com.topjohnwu.magisk.utils.HideTopViewOnScrollBehavior
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.reflect.KClass

open class MainActivity : CompatActivity<MainViewModel, ActivityMainMd2Binding>(),
    FragNavController.TransactionListener {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()
    override val navHost: Int = R.id.main_nav_host

    override val navigation by lazy { CompatNavigationDelegate(this, this) }

    override val baseFragments: List<KClass<out Fragment>> = listOf(
        HomeFragment::class,
        ModuleFragment::class,
        SuperuserFragment::class,
        LogFragment::class,
        SettingsFragment::class
    )

    //This temporarily fixes unwanted feature of BottomNavigationView - where the view applies
    //padding on itself given insets are not consumed beforehand. Unfortunately the listener
    //implementation doesn't favor us against the design library, so on re-create it's often given
    //upper hand.
    private val navObserver = ViewTreeObserver.OnGlobalLayoutListener {
        binding.mainNavigation.setPadding(0)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setSupportActionBar(binding.mainToolbar)

        binding.mainToolbarWrapper.updateLayoutParams<CoordinatorLayout.LayoutParams> {
            behavior = HideTopViewOnScrollBehavior<MaterialCardView>()
        }
        binding.mainNavigation.setOnNavigationItemSelectedListener {
            when (it.itemId) {
                R.id.homeFragment -> Navigation.home()
                R.id.modulesFragment -> Navigation.modules()
                R.id.superuserFragment -> Navigation.superuser()
                R.id.logFragment -> Navigation.log()
                R.id.settingsFragment -> Navigation.settings()
                else -> throw NotImplementedError("Id ${it.itemId} is not defined as selectable")
            }.dispatchOnSelf()
            true
        }

        binding.mainNavigation.viewTreeObserver.addOnGlobalLayoutListener(navObserver)

        if (intent.getBooleanExtra(Const.Key.OPEN_SETTINGS, false)) {
            binding.mainNavigation.selectedItemId = R.id.settingsFragment
        }
    }

    override fun onResume() {
        super.onResume()
        binding.mainNavigation.menu.apply {
            val isRoot = Shell.rootAccess()
            findItem(R.id.modulesFragment)?.isEnabled = isRoot
            findItem(R.id.superuserFragment)?.isEnabled = isRoot
            findItem(R.id.logFragment)?.isEnabled = isRoot
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

    override fun onTabTransaction(fragment: Fragment?, index: Int) {
        setDisplayHomeAsUpEnabled(false)
    }

    override fun onFragmentTransaction(
        fragment: Fragment?,
        transactionType: FragNavController.TransactionType
    ) {
        setDisplayHomeAsUpEnabled(!navigation.isRoot)

        val lapam = binding.mainBottomBar.layoutParams as ViewGroup.MarginLayoutParams
        val height = binding.mainBottomBar.measuredHeight
        val verticalMargin = lapam.let { it.topMargin + it.bottomMargin }
        val maxTranslation = height + verticalMargin
        val translation = if (navigation.isRoot) 0 else maxTranslation

        binding.mainBottomBar.animate()
            .translationY(translation.toFloat())
            .setInterpolator(FastOutSlowInInterpolator())
            .withStartAction { if (translation == 0) binding.mainBottomBar.isVisible = true }
            .withEndAction { if (translation > 0) binding.mainBottomBar.isVisible = false }
            .start()
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    fun setDisplayHomeAsUpEnabled(isEnabled: Boolean) {
        when {
            isEnabled -> binding.mainToolbar.setNavigationIcon(R.drawable.ic_back_md2)
            else -> binding.mainToolbar.navigationIcon = null
        }
    }

}