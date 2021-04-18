package com.topjohnwu.magisk.ui

import android.content.Intent
import android.content.pm.ApplicationInfo
import android.os.Bundle
import android.view.MenuItem
import android.view.View
import android.view.ViewTreeObserver
import android.view.WindowManager
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.core.view.forEach
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.ReselectionTarget
import com.topjohnwu.magisk.core.*
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.di.viewModel
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.ui.home.HomeFragmentDirections
import com.topjohnwu.magisk.utils.HideableBehavior
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.Shortcuts
import java.io.File

class MainViewModel : BaseViewModel()

open class MainActivity : BaseUIActivity<MainViewModel, ActivityMainMd2Binding>() {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()
    override val navHostId: Int = R.id.main_nav_host

    private var isRootFragment = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Make sure Splash is always ran before us
        if (!SplashActivity.DONE) {
            redirect<SplashActivity>().also { startActivity(it) }
            finish()
            return
        }

        setContentView()
        showUnsupportedMessage()
        askForHomeShortcut()

        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE)

        navigation?.addOnDestinationChangedListener { _, destination, _ ->
            isRootFragment = when (destination.id) {
                R.id.homeFragment,
                R.id.modulesFragment,
                R.id.superuserFragment,
                R.id.logFragment -> true
                else -> false
            }

            setDisplayHomeAsUpEnabled(!isRootFragment)
            requestNavigationHidden(!isRootFragment)

            binding.mainNavigation.menu.forEach {
                if (it.itemId == destination.id) {
                    it.isChecked = true
                }
            }
        }

        setSupportActionBar(binding.mainToolbar)

        binding.mainNavigation.setOnNavigationItemSelectedListener {
            getScreen(it.itemId)?.navigate()
            true
        }
        binding.mainNavigation.setOnNavigationItemReselectedListener {
            (currentFragment as? ReselectionTarget)?.onReselected()
        }

        val section = if (intent.action == Intent.ACTION_APPLICATION_PREFERENCES) Const.Nav.SETTINGS
        else intent.getStringExtra(Const.Key.OPEN_SECTION)
        getScreen(section)?.navigate()

        if (savedInstanceState != null) {
            if (!isRootFragment) {
                requestNavigationHidden()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        binding.mainNavigation.menu.apply {
            findItem(R.id.superuserFragment)?.isEnabled = Utils.showSuperUser()
            findItem(R.id.logFragment)?.isEnabled = Info.env.isActive
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            android.R.id.home -> onBackPressed()
            else -> return super.onOptionsItemSelected(item)
        }
        return true
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

        if (!binding.mainBottomBar.isAttachedToWindow) {
            binding.mainBottomBar.viewTreeObserver.addOnWindowAttachListener(object :
                ViewTreeObserver.OnWindowAttachListener {

                init {
                    val listener =
                        binding.mainBottomBar.tag as? ViewTreeObserver.OnWindowAttachListener
                    if (listener != null) {
                        binding.mainBottomBar.viewTreeObserver.removeOnWindowAttachListener(listener)
                    }
                    binding.mainBottomBar.tag = this
                }

                override fun onWindowAttached() {
                    requestNavigationHidden(hide)
                }

                override fun onWindowDetached() {
                }
            })
            return
        }

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
            Const.Nav.SUPERUSER -> MainDirections.actionSuperuserFragment()
            Const.Nav.HIDE -> MainDirections.actionHideFragment()
            Const.Nav.MODULES -> MainDirections.actionModuleFragment()
            Const.Nav.SETTINGS -> HomeFragmentDirections.actionHomeFragmentToSettingsFragment()
            else -> null
        }
    }

    private fun getScreen(id: Int): NavDirections? {
        return when (id) {
            R.id.homeFragment -> MainDirections.actionHomeFragment()
            R.id.modulesFragment -> MainDirections.actionModuleFragment()
            R.id.superuserFragment -> MainDirections.actionSuperuserFragment()
            R.id.logFragment -> MainDirections.actionLogFragment()
            else -> null
        }
    }

    private fun showUnsupportedMessage() {
        if (Info.env.isUnsupported) {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_magisk_title)
                .applyMessage(R.string.unsupport_magisk_msg, Const.Version.MIN_VERSION)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) { titleRes = android.R.string.ok }
                .cancellable(false)
                .reveal()
        }

        if (!Info.isEmulator && Info.env.isActive && System.getenv("PATH")
                ?.split(':')
                ?.filterNot { File("$it/magisk").exists() }
                ?.any { File("$it/su").exists() } == true) {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_general_title)
                .applyMessage(R.string.unsupport_other_su_msg)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) { titleRes = android.R.string.ok }
                .cancellable(false)
                .reveal()
        }

        if (applicationInfo.flags and ApplicationInfo.FLAG_SYSTEM != 0) {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_general_title)
                .applyMessage(R.string.unsupport_system_app_msg)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) { titleRes = android.R.string.ok }
                .cancellable(false)
                .reveal()
        }

        if (applicationInfo.flags and ApplicationInfo.FLAG_EXTERNAL_STORAGE != 0) {
            MagiskDialog(this)
                .applyTitle(R.string.unsupport_general_title)
                .applyMessage(R.string.unsupport_external_storage_msg)
                .applyButton(MagiskDialog.ButtonType.POSITIVE) { titleRes = android.R.string.ok }
                .cancellable(false)
                .reveal()
        }
    }

    private fun askForHomeShortcut() {
        if (isRunningAsStub && !Config.askedHome &&
            ShortcutManagerCompat.isRequestPinShortcutSupported(this)) {
            // Ask and show dialog
            Config.askedHome = true
            MagiskDialog(this)
                .applyTitle(R.string.add_shortcut_title)
                .applyMessage(R.string.add_shortcut_msg)
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = android.R.string.cancel
                }.applyButton(MagiskDialog.ButtonType.POSITIVE) {
                    titleRes = android.R.string.ok
                    onClick {
                        Shortcuts.addHomeIcon(this@MainActivity)
                    }
                }.cancellable(true)
                .reveal()
        }
    }
}
