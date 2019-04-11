package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.view.MenuItem
import android.view.View
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.fragment.app.Fragment
import com.google.android.material.navigation.NavigationView
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainBinding
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.MagiskFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import kotlinx.android.synthetic.main.toolbar.*
import org.koin.androidx.viewmodel.ext.android.viewModel

open class MainActivity : MagiskActivity<MainViewModel, ActivityMainBinding>(),
    NavigationView.OnNavigationItemSelectedListener {

    override val layoutRes: Int = R.layout.activity_main
    override val viewModel: MainViewModel by viewModel()
    override val navHostId: Int = R.id.main_nav_host

    private val mDrawerHandler = Handler()
    private var mDrawerItem: Int = 0
    private var toolbarElevation: Float = 0.toFloat()

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
        setSupportActionBar(toolbar)

        val toggle = object :
            ActionBarDrawerToggle(
                this,
                binding.drawerLayout,
                toolbar,
                R.string.magisk,
                R.string.magisk
            ) {
            override fun onDrawerOpened(drawerView: View) {
                super.onDrawerOpened(drawerView)
                super.onDrawerSlide(drawerView, 0f) // this disables the arrow @ completed tate
            }

            override fun onDrawerSlide(drawerView: View, slideOffset: Float) {
                super.onDrawerSlide(drawerView, 0f) // this disables the animation
            }
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            toolbarElevation = toolbar!!.elevation
        }

        binding.drawerLayout.addDrawerListener(toggle)
        toggle.syncState()

        if (savedInstanceState == null) {
            val section = intent.getStringExtra(Const.Key.OPEN_SECTION)
            fromShortcut = section != null
            navigate(section)
        }

        binding.navView.setNavigationItemSelectedListener(this)
    }

    override fun onBackPressed() {
        if (binding.drawerLayout.isDrawerOpen(binding.navView)) {
            binding.drawerLayout.closeDrawer(binding.navView)
        } else if (mDrawerItem != R.id.magisk && !fromShortcut) {
            navigate(R.id.magisk)
        } else {
            finish()
        }
    }

    override fun onNavigationItemSelected(menuItem: MenuItem): Boolean {
        mDrawerHandler.removeCallbacksAndMessages(null)
        mDrawerHandler.postDelayed({ navigate(menuItem.itemId) }, 250)
        binding.drawerLayout.closeDrawer(binding.navView)
        return true
    }

    fun checkHideSection() {
        val menu = binding.navView.menu
        menu.findItem(R.id.magiskhide).isVisible =
            Shell.rootAccess() && Config.get<Any>(Config.Key.MAGISKHIDE) as Boolean
        menu.findItem(R.id.modules).isVisible = Shell.rootAccess() && Config.magiskVersionCode >= 0
        menu.findItem(R.id.downloads).isVisible = (Networking.checkNetworkStatus(this)
                && Shell.rootAccess() && Config.magiskVersionCode >= 0)
        menu.findItem(R.id.log).isVisible = Shell.rootAccess()
        menu.findItem(R.id.superuser).isVisible = Utils.showSuperUser()
    }

    fun navigate(item: String?) {
        var itemId = R.id.magisk
        if (item != null) {
            when (item) {
                "superuser" -> itemId = R.id.superuser
                "modules" -> itemId = R.id.modules
                "downloads" -> itemId = R.id.downloads
                "magiskhide" -> itemId = R.id.magiskhide
                "log" -> itemId = R.id.log
                "settings" -> itemId = R.id.settings
            }
        }
        navigate(itemId)
    }

    fun navigate(itemId: Int) {
        mDrawerItem = itemId
        binding.navView.setCheckedItem(itemId)
        when (itemId) {
            R.id.magisk -> {
                fromShortcut = false
                displayFragment(MagiskFragment(), true)
            }
            R.id.superuser -> displayFragment(SuperuserFragment(), true)
            R.id.modules -> displayFragment(ModulesFragment(), true)
            R.id.downloads -> displayFragment(ReposFragment(), true)
            R.id.magiskhide -> displayFragment(MagiskHideFragment(), true)
            R.id.log -> displayFragment(LogFragment(), false)
            R.id.settings -> displayFragment(SettingsFragment(), true)
        }
    }

    @Deprecated("")
    private fun displayFragment(navFragment: Fragment, setElevation: Boolean) {
        /*supportInvalidateOptionsMenu();
        getSupportFragmentManager()
                .beginTransaction()
                .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                .replace(R.id.content_frame, navFragment)
                .commitNow();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            toolbar.setElevation(setElevation ? toolbarElevation : 0);
        }*/
    }

    companion object {
        private var fromShortcut = false
    }
}
