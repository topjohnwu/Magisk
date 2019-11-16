package com.topjohnwu.magisk.ui

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AlertDialog
import androidx.core.view.GravityCompat
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentTransaction
import com.ncapdevi.fragnav.FragNavController
import com.ncapdevi.fragnav.FragNavTransactionOptions
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Const.Key.OPEN_SECTION
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.base.BaseActivity
import com.topjohnwu.magisk.base.BaseFragment
import com.topjohnwu.magisk.databinding.ActivityMainBinding
import com.topjohnwu.magisk.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.intent
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.navigation.MagiskAnimBuilder
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.model.navigation.Navigator
import com.topjohnwu.magisk.ui.hide.MagiskHideFragment
import com.topjohnwu.magisk.ui.home.HomeFragment
import com.topjohnwu.magisk.ui.log.LogFragment
import com.topjohnwu.magisk.ui.module.ModulesFragment
import com.topjohnwu.magisk.ui.module.ReposFragment
import com.topjohnwu.magisk.ui.settings.SettingsFragment
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment
import com.topjohnwu.magisk.utils.Utils
import org.koin.androidx.viewmodel.ext.android.viewModel
import timber.log.Timber
import kotlin.reflect.KClass

open class MainActivity : BaseActivity<MainViewModel, ActivityMainBinding>(), Navigator,
        FragNavController.RootFragmentListener, FragNavController.TransactionListener {

    override val layoutRes: Int = R.layout.activity_main
    override val viewModel: MainViewModel by viewModel()
    private val navHostId: Int = R.id.main_nav_host
    private val defaultPosition: Int = 0

    private val navigationController by lazy {
        FragNavController(supportFragmentManager, navHostId)
    }
    private val isRootFragment get() =
        navigationController.currentStackIndex != defaultPosition

    override val baseFragments: List<KClass<out Fragment>> = listOf(
        HomeFragment::class,
        SuperuserFragment::class,
        MagiskHideFragment::class,
        ModulesFragment::class,
        ReposFragment::class,
        LogFragment::class,
        SettingsFragment::class
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        if (!SplashActivity.DONE) {
            startActivity(intent<SplashActivity>())
            finish()
        }

        super.onCreate(savedInstanceState)

        if (Info.env.isUnsupported && !viewModel.shownUnsupportedDialog) {
            viewModel.shownUnsupportedDialog = true
            AlertDialog.Builder(this)
                .setTitle(R.string.unsupport_magisk_title)
                .setMessage(getString(R.string.unsupport_magisk_msg, Const.Version.MIN_VERSION))
                .setPositiveButton(android.R.string.ok, null)
                .show()
        }

        navigationController.apply {
            rootFragmentListener = this@MainActivity
            transactionListener = this@MainActivity
            initialize(defaultPosition, savedInstanceState)
        }

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

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        navigationController.onSaveInstanceState(outState)
    }

    override fun setTitle(title: CharSequence?) {
        supportActionBar?.title = title
    }

    override fun setTitle(titleId: Int) {
        supportActionBar?.setTitle(titleId)
    }

    override fun onBackPressed() {
        if (binding.drawerLayout.isDrawerOpen(binding.navView)) {
            binding.drawerLayout.closeDrawer(binding.navView)
        } else {
            val fragment = navigationController.currentFrag as? BaseFragment<*, *>

            if (fragment?.onBackPressed() == true) {
                return
            }

            try {
                navigationController.popFragment()
            } catch (e: UnsupportedOperationException) {
                when {
                    isRootFragment -> {
                        val options = FragNavTransactionOptions.newBuilder()
                                .transition(FragmentTransaction.TRANSIT_FRAGMENT_CLOSE)
                                .build()
                        navigationController.switchTab(defaultPosition, options)
                    }
                    else -> super.onBackPressed()
                }
            }
        }
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(this), event.length, event.f)
            is BackPressEvent -> onBackPressed()
            is MagiskNavigationEvent -> navigateTo(event)
            is ViewActionEvent -> event.action(this)
            is PermissionEvent -> withPermissions(*event.permissions.toTypedArray()) {
                onSuccess { event.callback.onNext(true) }
                onFailure {
                    event.callback.onNext(false)
                    event.callback.onError(SecurityException("User refused permissions"))
                }
            }
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
        menu.findItem(R.id.magiskHideFragment).isVisible = Info.env.isActive && Info.env.magiskHide
        menu.findItem(R.id.modulesFragment).isVisible = Info.env.isActive
        menu.findItem(R.id.reposFragment).isVisible = Info.isConnected.value && Info.env.isActive
        menu.findItem(R.id.logFragment).isVisible = Info.env.isActive
        menu.findItem(R.id.superuserFragment).isVisible = Utils.showSuperUser()
    }

    private fun FragNavTransactionOptions.Builder.customAnimations(options: MagiskAnimBuilder) =
            customAnimations(options.enter, options.exit, options.popEnter, options.popExit).apply {
                if (!options.anySet) {
                    transition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                }
            }

    override val numberOfRootFragments: Int get() = baseFragments.size

    override fun getRootFragment(index: Int) = baseFragments[index].java.newInstance()

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

    override fun navigateTo(event: MagiskNavigationEvent) {
        val directions = event.navDirections

        navigationController.defaultTransactionOptions = FragNavTransactionOptions.newBuilder()
                .customAnimations(event.animOptions)
                .build()

        navigationController.currentStack
                ?.indexOfFirst { it.javaClass == event.navOptions.popUpTo }
                ?.let { if (it == -1) null else it } // invalidate if class is not found
                ?.let { if (event.navOptions.inclusive) it + 1 else it }
                ?.let { navigationController.popFragments(it) }

        when (directions.isActivity) {
            true -> navigateToActivity(event)
            else -> navigateToFragment(event)
        }
    }

    private fun navigateToActivity(event: MagiskNavigationEvent) {
        val destination = event.navDirections.destination?.java ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }
        val options = event.navOptions

        Intent(this, destination)
                .putExtras(event.navDirections.args)
                .apply {
                    if (options.singleTop) addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
                    if (options.clearTask) addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
                }
                .let { startActivity(it) }
    }

    private fun navigateToFragment(event: MagiskNavigationEvent) {
        val destination = event.navDirections.destination?.java ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }

        when (val index = baseFragments.indexOfFirst { it.java.name == destination.name }) {
            -1 -> destination.newInstance()
                    .apply { arguments = event.navDirections.args }
                    .let { navigationController.pushFragment(it) }
            // When it's desired that fragments of same class are put on top of one another edit this
            else -> navigationController.switchTab(index)
        }
    }

    override fun onFragmentTransaction(
            fragment: Fragment?,
            transactionType: FragNavController.TransactionType
    ) = Unit
}
