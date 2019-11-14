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
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatActivity
import com.topjohnwu.magisk.redesign.compat.CompatNavigationDelegate
import com.topjohnwu.magisk.redesign.home.HomeFragment
import com.topjohnwu.magisk.redesign.module.ModuleFragment
import com.topjohnwu.magisk.redesign.superuser.SuperuserFragment
import com.topjohnwu.magisk.utils.HideBottomViewOnScrollBehavior
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
        SuperuserFragment::class
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
        binding.mainBottomBar.updateLayoutParams<CoordinatorLayout.LayoutParams> {
            behavior = HideBottomViewOnScrollBehavior<MaterialCardView>()
        }
        binding.mainNavigation.setOnNavigationItemSelectedListener {
            when (it.itemId) {
                R.id.homeFragment -> Navigation.home()
                R.id.modulesFragment -> Navigation.modules()
                R.id.superuserFragment -> Navigation.superuser()
                else -> throw NotImplementedError("Id ${it.itemId} is not defined as selectable")
            }.dispatchOnSelf()
            true
        }

        binding.mainNavigation.viewTreeObserver.addOnGlobalLayoutListener(navObserver)

        if (intent.getBooleanExtra(Const.Key.OPEN_SETTINGS, false)) {
            Navigation.settings().dispatchOnSelf()
        }

        if (savedInstanceState != null) {
            onTabTransaction(null, -1)
            onFragmentTransaction(null, FragNavController.TransactionType.PUSH)
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

    override fun onTabTransaction(fragment: Fragment?, index: Int) {
        setDisplayHomeAsUpEnabled(false)
    }

    override fun onFragmentTransaction(
        fragment: Fragment?,
        transactionType: FragNavController.TransactionType
    ) {
        setDisplayHomeAsUpEnabled(!navigation.isRoot)
        requestNavigationHidden(!navigation.isRoot)
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    private fun setDisplayHomeAsUpEnabled(isEnabled: Boolean) {
        binding.mainToolbar.startAnimations()
        when {
            isEnabled -> binding.mainToolbar.setNavigationIcon(R.drawable.ic_back_md2)
            else -> binding.mainToolbar.navigationIcon = null
        }
    }

    internal fun requestNavigationHidden(hide: Boolean = true) {
        val lapam = binding.mainBottomBar.layoutParams as ViewGroup.MarginLayoutParams
        val height = binding.mainBottomBar.measuredHeight
        val verticalMargin = lapam.let { it.topMargin + it.bottomMargin }
        val maxTranslation = height + verticalMargin
        val translation = if (!hide) 0 else maxTranslation

        binding.mainBottomBar.animate()
            .translationY(translation.toFloat())
            .setInterpolator(FastOutSlowInInterpolator())
            .withStartAction { if (!hide) binding.mainBottomBar.isVisible = true }
            .withEndAction { if (hide) binding.mainBottomBar.isVisible = false }
            .start()
    }

}