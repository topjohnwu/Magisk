package com.topjohnwu.magisk.redesign

import android.graphics.Insets
import android.os.Bundle
import androidx.fragment.app.Fragment
import com.ncapdevi.fragnav.FragNavController
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatActivity
import com.topjohnwu.magisk.redesign.home.HomeFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.reflect.KClass

open class MainActivity : CompatActivity<MainViewModel, ActivityMainMd2Binding>() {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()
    override val navHostId: Int = R.id.main_nav_host
    override val defaultPosition: Int = 0

    override val baseFragments: List<KClass<out Fragment>> = listOf(
        HomeFragment::class,
        ModulesFragment::class,
        SuperuserFragment::class,
        LogFragment::class,
        SettingsFragment::class
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setSupportActionBar(binding.mainToolbar)

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

        if (intent.getBooleanExtra(Const.Key.OPEN_SETTINGS, false)) {
            binding.mainNavigation.selectedItemId = R.id.settingsFragment
        }
    }

    override fun onTabTransaction(fragment: Fragment?, index: Int) {
        super.onTabTransaction(fragment, index)

        setDisplayHomeAsUpEnabled(false)
    }

    override fun onFragmentTransaction(
        fragment: Fragment?,
        transactionType: FragNavController.TransactionType
    ) {
        super.onFragmentTransaction(fragment, transactionType)

        when (transactionType) {
            FragNavController.TransactionType.PUSH -> setDisplayHomeAsUpEnabled(true)
            else -> Unit //dunno might be useful
        }
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