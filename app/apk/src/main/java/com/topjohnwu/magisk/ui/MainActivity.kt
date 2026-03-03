package com.topjohnwu.magisk.ui

import android.Manifest
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.annotation.SuppressLint
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.net.Uri
import android.os.Bundle
import android.view.View
import android.view.WindowManager
import android.widget.Toast
import androidx.activity.compose.setContent
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.core.net.toUri
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.viewmodel.navigation3.rememberViewModelStoreNavEntryDecorator
import androidx.navigation3.runtime.NavEntry
import androidx.navigation3.runtime.entryProvider
import androidx.navigation3.runtime.rememberSaveableStateHolderNavEntryDecorator
import androidx.navigation3.ui.NavDisplay
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.SplashController
import com.topjohnwu.magisk.core.base.SplashScreenHost
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.ui.deny.DenyListScreen
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.ui.flash.FlashScreen
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.install.InstallScreen
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.module.ActionScreen
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.ui.navigation.CollectNavEvents
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Navigator
import com.topjohnwu.magisk.ui.navigation.ObserveViewEvents
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.navigation.rememberNavigator
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MagiskDialogHost
import com.topjohnwu.magisk.view.Shortcuts
import top.yukonga.miuix.kmp.utils.MiuixPopupUtils.Companion.MiuixPopupHost
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.ui.Modifier
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.launch
import androidx.compose.runtime.Composable
import com.topjohnwu.magisk.core.R as CoreR

class MainViewModel : BaseViewModel()

class MainActivity : UIActivity(), SplashScreenHost {

    override val viewModel by viewModel<MainViewModel>()
    override val splashController = SplashController(this)
    override val snackbarView: View
        get() = window.decorView.findViewById(android.R.id.content)
    override val snackbarAnchorView: View? get() = null

    private val intentState = MutableStateFlow(0)

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(Theme.selected.themeRes)
        splashController.preOnCreate()
        super.onCreate(savedInstanceState)
        splashController.onCreate(savedInstanceState)
    }

    override fun onResume() {
        super.onResume()
        splashController.onResume()
    }

    @SuppressLint("InlinedApi")
    override fun onCreateUi(savedInstanceState: Bundle?) {
        showUnsupportedMessage()
        askForHomeShortcut()

        if (Config.checkUpdate) {
            withPermission(Manifest.permission.POST_NOTIFICATIONS) {
                Config.checkUpdate = it
            }
        }

        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE)

        val initialTab = getInitialTab(intent)

        setContent {
            MagiskTheme {
                Box(modifier = Modifier.fillMaxSize()) {
                    val navigator = rememberNavigator(Route.Main)
                    CompositionLocalProvider(LocalNavigator provides navigator) {
                        HandleFlashIntent(navigator)

                        NavDisplay(
                            backStack = navigator.backStack,
                            onBack = { navigator.pop() },
                            entryDecorators = listOf(
                                rememberSaveableStateHolderNavEntryDecorator(),
                                rememberViewModelStoreNavEntryDecorator()
                            ),
                            entryProvider = entryProvider {
                                entry<Route.Main> {
                                    MainScreen(initialTab = initialTab)
                                }
                                entry<Route.Install> { _ ->
                                    val vm: InstallViewModel = androidx.lifecycle.viewmodel.compose.viewModel(factory = VMFactory)
                                    ObserveViewEvents(vm)
                                    CollectNavEvents(vm, navigator)
                                    InstallScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.DenyList> { _ ->
                                    val vm: DenyListViewModel = androidx.lifecycle.viewmodel.compose.viewModel(factory = VMFactory)
                                    LaunchedEffect(Unit) { vm.startLoading() }
                                    ObserveViewEvents(vm)
                                    DenyListScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.Flash> { key ->
                                    val vm: FlashViewModel = androidx.lifecycle.viewmodel.compose.viewModel(factory = VMFactory)
                                    LaunchedEffect(key) {
                                        if (vm.flashAction.isEmpty()) {
                                            vm.flashAction = key.action
                                            vm.flashUri = key.additionalData?.let { Uri.parse(it) }
                                            vm.startFlashing()
                                        }
                                    }
                                    ObserveViewEvents(vm)
                                    FlashScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.Action> { key ->
                                    val vm: ActionViewModel = androidx.lifecycle.viewmodel.compose.viewModel(factory = VMFactory)
                                    LaunchedEffect(key) {
                                        if (vm.actionId.isEmpty()) {
                                            vm.actionId = key.id
                                            vm.actionName = key.name
                                            vm.startRunAction()
                                        }
                                    }
                                    ObserveViewEvents(vm)
                                    ActionScreen(vm, actionName = key.name, onBack = { navigator.pop() })
                                }
                            }
                        )
                    }
                    MagiskDialogHost()
                    MiuixPopupHost()
                }
            }
        }
    }

    @Composable
    private fun HandleFlashIntent(navigator: Navigator) {
        val intentVersion by intentState.collectAsState()
        LaunchedEffect(intentVersion) {
            val currentIntent = intent ?: return@LaunchedEffect
            if (currentIntent.action == FlashUtils.INTENT_FLASH) {
                val action = currentIntent.getStringExtra(FlashUtils.EXTRA_FLASH_ACTION)
                    ?: return@LaunchedEffect
                val uri = currentIntent.getStringExtra(FlashUtils.EXTRA_FLASH_URI)
                navigator.push(Route.Flash(action, uri))
                currentIntent.action = null
            }
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        intentState.value += 1
    }

    private fun getInitialTab(intent: Intent?): Int {
        val section = if (intent?.action == Intent.ACTION_APPLICATION_PREFERENCES) {
            Const.Nav.SETTINGS
        } else {
            intent?.getStringExtra(Const.Key.OPEN_SECTION)
        }
        return when (section) {
            Const.Nav.SUPERUSER -> Tab.SUPERUSER.ordinal
            Const.Nav.MODULES -> Tab.MODULES.ordinal
            Const.Nav.SETTINGS -> Tab.SETTINGS.ordinal
            else -> Tab.HOME.ordinal
        }
    }

    @SuppressLint("InlinedApi")
    override fun showInvalidStateMessage(): Unit = runOnUiThread {
        MagiskDialog(this).apply {
            setTitle(CoreR.string.unsupport_nonroot_stub_title)
            setMessage(CoreR.string.unsupport_nonroot_stub_msg)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = CoreR.string.install
                onClick {
                    withPermission(REQUEST_INSTALL_PACKAGES) {
                        if (!it) {
                            toast(CoreR.string.install_unknown_denied, Toast.LENGTH_SHORT)
                            showInvalidStateMessage()
                        } else {
                            lifecycleScope.launch {
                                AppMigration.restore(this@MainActivity)
                            }
                        }
                    }
                }
            }
            setCancelable(false)
            show()
        }
    }

    private fun showUnsupportedMessage() {
        if (Info.env.isUnsupported) {
            MagiskDialog(this).apply {
                setTitle(CoreR.string.unsupport_magisk_title)
                setMessage(CoreR.string.unsupport_magisk_msg, Const.Version.MIN_VERSION)
                setButton(MagiskDialog.ButtonType.POSITIVE) { text = android.R.string.ok }
                setCancelable(false)
            }.show()
        }

        if (!Info.isEmulator && Info.env.isActive && System.getenv("PATH")
                ?.split(':')
                ?.filterNot { java.io.File("$it/magisk").exists() }
                ?.any { java.io.File("$it/su").exists() } == true) {
            MagiskDialog(this).apply {
                setTitle(CoreR.string.unsupport_general_title)
                setMessage(CoreR.string.unsupport_other_su_msg)
                setButton(MagiskDialog.ButtonType.POSITIVE) { text = android.R.string.ok }
                setCancelable(false)
            }.show()
        }

        if (applicationInfo.flags and ApplicationInfo.FLAG_SYSTEM != 0) {
            MagiskDialog(this).apply {
                setTitle(CoreR.string.unsupport_general_title)
                setMessage(CoreR.string.unsupport_system_app_msg)
                setButton(MagiskDialog.ButtonType.POSITIVE) { text = android.R.string.ok }
                setCancelable(false)
            }.show()
        }

        if (applicationInfo.flags and ApplicationInfo.FLAG_EXTERNAL_STORAGE != 0) {
            MagiskDialog(this).apply {
                setTitle(CoreR.string.unsupport_general_title)
                setMessage(CoreR.string.unsupport_external_storage_msg)
                setButton(MagiskDialog.ButtonType.POSITIVE) { text = android.R.string.ok }
                setCancelable(false)
            }.show()
        }
    }

    private fun askForHomeShortcut() {
        if (isRunningAsStub && !Config.askedHome &&
            ShortcutManagerCompat.isRequestPinShortcutSupported(this)) {
            Config.askedHome = true
            MagiskDialog(this).apply {
                setTitle(CoreR.string.add_shortcut_title)
                setMessage(CoreR.string.add_shortcut_msg)
                setButton(MagiskDialog.ButtonType.NEGATIVE) {
                    text = android.R.string.cancel
                }
                setButton(MagiskDialog.ButtonType.POSITIVE) {
                    text = android.R.string.ok
                    onClick {
                        Shortcuts.addHomeIcon(this@MainActivity)
                    }
                }
                setCancelable(true)
            }.show()
        }
    }
}
