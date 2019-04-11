package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Bundle
import androidx.core.view.GravityCompat
import androidx.navigation.ui.setupWithNavController
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainBinding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel

open class MainActivity : MagiskActivity<MainViewModel, ActivityMainBinding>() {

    override val layoutRes: Int = R.layout.activity_main
    override val viewModel: MainViewModel by viewModel()
    override val navHostId: Int = R.id.main_nav_host

    /*override fun getDarkTheme(): Int {
        return R.style.AppTheme_Dark
    }*/

    override fun onCreate(savedInstanceState: Bundle?) {
        if (!SplashActivity.DONE) {
            startActivity(Intent(this, ClassMap.get<Any>(SplashActivity::class.java)))
            finish()
        }

        super.onCreate(savedInstanceState)
        checkHideSection()
        setSupportActionBar(binding.mainInclude.mainToolbar)

        binding.navView.setupWithNavController(navController)
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

    fun checkHideSection() {
        val menu = binding.navView.menu
        menu.findItem(R.id.magiskHideFragment).isVisible =
            Shell.rootAccess() && Config.get<Any>(Config.Key.MAGISKHIDE) as Boolean
        menu.findItem(R.id.modulesFragment).isVisible =
            Shell.rootAccess() && Config.magiskVersionCode >= 0
        menu.findItem(R.id.reposFragment).isVisible =
            (Networking.checkNetworkStatus(this) && Shell.rootAccess() && Config.magiskVersionCode >= 0)
        menu.findItem(R.id.logFragment).isVisible =
            Shell.rootAccess()
        menu.findItem(R.id.superuserFragment).isVisible =
            Utils.showSuperUser()
    }
}
