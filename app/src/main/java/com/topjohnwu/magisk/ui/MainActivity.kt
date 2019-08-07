package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Bundle
import androidx.core.view.GravityCompat
import androidx.fragment.app.Fragment
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const.Key.OPEN_SECTION
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainBinding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.HomeFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.reflect.KClass


open class MainActivity : MagiskActivity<MainViewModel, ActivityMainBinding>() {

    override val layoutRes: Int = R.layout.activity_main
    override val viewModel: MainViewModel by viewModel()
    override val navHostId: Int = R.id.main_nav_host
    override val defaultPosition: Int = 0

    override val baseFragments: List<KClass<out Fragment>> = listOf(
        HomeFragment::class,
        SuperuserFragment::class,
        MagiskHideFragment::class,
        ModulesFragment::class,
        ReposFragment::class,
        LogFragment::class,
        SettingsFragment::class
    )

    /*override fun getDarkTheme(): Int {
        return R.style.AppTheme_Dark
    }*/

    override fun onCreate(savedInstanceState: Bundle?) {
        if (!SplashActivity.DONE) {
            startActivity(Intent(this, ClassMap[SplashActivity::class.java]))
            finish()
        }

        super.onCreate(savedInstanceState)
        checkHideSection()
        setSupportActionBar(binding.mainInclude.mainToolbar)

        viewModel.isConnected.addOnPropertyChangedCallback {
            checkHideSection()
        }

        if (savedInstanceState == null) {
            intent.getStringExtra(OPEN_SECTION)?.let {
                onEventDispatched(Navigation.fromSection(it))
            }
        }
    }

    override fun setTitle(title: CharSequence?) {
        supportActionBar?.title = title
    }

    override fun setTitle(titleId: Int) {
        supportActionBar?.setTitle(titleId)
    }

    override fun onTabTransaction(fragment: Fragment?, index: Int) {
        val fragmentId = when (fragment) {
            is HomeFragment -> R.id.magiskFragment
            is SuperuserFragment -> R.id.superuserFragment
            is MagiskHideFragment -> R.id.magiskHideFragment
            is ModulesFragment -> R.id.modulesFragment
            is ReposFragment -> R.id.reposFragment
            is LogFragment -> R.id.logFragment
            is SettingsFragment -> R.id.settings
            else -> return
        }
        binding.navView.setCheckedItem(fragmentId)
    }

    override fun onBackPressed() {
        if (binding.drawerLayout.isDrawerOpen(binding.navView)) {
            binding.drawerLayout.closeDrawer(binding.navView)
        } else {
            super.onBackPressed()
        }
    }

    override fun onSimpleEventDispatched(event: Int) {
        super.onSimpleEventDispatched(event)
        when (event) {
            Navigation.Main.OPEN_NAV -> openNav()
        }
    }

    private fun openNav() = binding.drawerLayout.openDrawer(GravityCompat.START)

    private fun checkHideSection() {
        val menu = binding.navView.menu
        menu.findItem(R.id.magiskHideFragment).isVisible =
            Shell.rootAccess() && Config.magiskHide
        menu.findItem(R.id.modulesFragment).isVisible =
            Shell.rootAccess() && Info.magiskVersionCode >= 0
        menu.findItem(R.id.reposFragment).isVisible =
            (viewModel.isConnected.value && Shell.rootAccess() && Info.magiskVersionCode >= 0)
        menu.findItem(R.id.logFragment).isVisible =
            Shell.rootAccess()
        menu.findItem(R.id.superuserFragment).isVisible =
            Utils.showSuperUser()
    }
}
