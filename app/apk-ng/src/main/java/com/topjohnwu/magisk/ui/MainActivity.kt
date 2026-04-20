package com.topjohnwu.magisk.ui

import android.Manifest
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.net.Uri
import android.os.Bundle
import androidx.core.net.toUri
import android.view.WindowManager
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Modifier
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.viewmodel.navigation3.rememberViewModelStoreNavEntryDecorator
import androidx.navigation3.runtime.entryProvider
import androidx.navigation3.runtime.rememberSaveableStateHolderNavEntryDecorator
import androidx.navigation3.ui.NavDisplay
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.ActivityExtension
import com.topjohnwu.magisk.core.base.SplashController
import com.topjohnwu.magisk.core.base.SplashScreenHost
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ui.deny.DenyListScreen
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.ui.flash.FlashScreen
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.module.ActionScreen
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Navigator
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.navigation.rememberNavigator
import com.topjohnwu.magisk.ui.superuser.SuperuserDetailScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.view.Shortcuts
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.launch
import com.topjohnwu.magisk.core.R as CoreR

class MainActivity : ComponentActivity(), SplashScreenHost {

    override val extension = ActivityExtension(this)
    override val splashController = SplashController(this)

    private val intentState = MutableStateFlow(0)
    internal val showInvalidState = MutableStateFlow(false)
    internal val showUnsupported = MutableStateFlow<List<Pair<Int, Int>>>(emptyList())
    internal val showShortcutPrompt = MutableStateFlow(false)

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        extension.onCreate(savedInstanceState)
        splashController.preOnCreate()
        theme.applyStyle(R.style.Main, true)
        super.onCreate(savedInstanceState)
        splashController.onCreate(savedInstanceState)
    }

    override fun onResume() {
        super.onResume()
        splashController.onResume()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        extension.onSaveInstanceState(outState)
    }

    @SuppressLint("InlinedApi")
    override fun onCreateUi(savedInstanceState: Bundle?) {
        showUnsupportedMessage()
        askForHomeShortcut()

        if (Config.checkUpdate) {
            extension.withPermission(Manifest.permission.POST_NOTIFICATIONS) {
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
                                rememberViewModelStoreNavEntryDecorator<Any>()
                            ),
                            entryProvider = entryProvider {
                                entry<Route.Main> {
                                    MainScreen(initialTab = initialTab)
                                }
                                entry<Route.DenyList> { _ ->
                                    val vm: DenyListViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(Unit) { vm.startLoading() }
                                    DenyListScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.Flash> { key ->
                                    val vm: FlashViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(key) {
                                        if (vm.flashAction.isEmpty()) {
                                            vm.flashAction = key.action
                                            vm.flashUri = key.additionalData?.toUri()
                                            vm.startFlashing()
                                        }
                                    }
                                    FlashScreen(vm, action = key.action, onBack = { navigator.pop() })
                                }
                                entry<Route.SuperuserDetail> { key ->
                                    val vm: SuperuserViewModel = viewModel(
                                        viewModelStoreOwner = this@MainActivity, factory = VMFactory
                                    )
                                    LaunchedEffect(Unit) {
                                        vm.authenticate = { onSuccess ->
                                            extension.withAuthentication { if (it) onSuccess() }
                                        }
                                    }
                                    SuperuserDetailScreen(uid = key.uid, viewModel = vm, onBack = { navigator.pop() })
                                }
                                entry<Route.Action> { key ->
                                    val vm: ActionViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(key) {
                                        if (vm.actionId.isEmpty()) {
                                            vm.actionId = key.id
                                            vm.actionName = key.name
                                            vm.startRunAction()
                                        }
                                    }
                                    ActionScreen(vm, actionName = key.name, onBack = { navigator.pop() })
                                }
                            }
                        )
                    }
                    MainActivityDialogs(activity = this@MainActivity)
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
    override fun showInvalidStateMessage() {
        showInvalidState.value = true
    }

    internal fun handleInvalidStateInstall() {
        extension.withPermission(REQUEST_INSTALL_PACKAGES) {
            if (!it) {
                toast(CoreR.string.install_unknown_denied, Toast.LENGTH_SHORT)
                showInvalidState.value = true
            } else {
                lifecycleScope.launch {
                    if (!AppMigration.restoreApp(this@MainActivity)) {
                        toast(CoreR.string.failure, Toast.LENGTH_LONG)
                    }
                }
            }
        }
    }

    private fun showUnsupportedMessage() {
        val messages = mutableListOf<Pair<Int, Int>>()

        if (Info.env.isUnsupported) {
            messages.add(CoreR.string.unsupport_magisk_title to CoreR.string.unsupport_magisk_msg)
        }
        if (!Info.isEmulator && Info.env.isActive && System.getenv("PATH")
                ?.split(':')
                ?.filterNot { java.io.File("$it/magisk").exists() }
                ?.any { java.io.File("$it/su").exists() } == true) {
            messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_other_su_msg)
        }
        if (applicationInfo.flags and ApplicationInfo.FLAG_SYSTEM != 0) {
            messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_system_app_msg)
        }
        if (applicationInfo.flags and ApplicationInfo.FLAG_EXTERNAL_STORAGE != 0) {
            messages.add(CoreR.string.unsupport_general_title to CoreR.string.unsupport_external_storage_msg)
        }

        if (messages.isNotEmpty()) {
            showUnsupported.value = messages
        }
    }

    private fun askForHomeShortcut() {
        if (isRunningAsStub && !Config.askedHome &&
            ShortcutManagerCompat.isRequestPinShortcutSupported(this)) {
            Config.askedHome = true
            showShortcutPrompt.value = true
        }
    }
}

@Composable
private fun MainActivityDialogs(activity: MainActivity) {
    val showInvalid by activity.showInvalidState.collectAsState()
    val unsupportedMessages by activity.showUnsupported.collectAsState()
    val showShortcut by activity.showShortcutPrompt.collectAsState()

    val invalidDialog = com.topjohnwu.magisk.ui.component.rememberConfirmDialog(
        onConfirm = {
            activity.showInvalidState.value = false
            activity.handleInvalidStateInstall()
        },
        onDismiss = {}
    )

    LaunchedEffect(showInvalid) {
        if (showInvalid) {
            invalidDialog.showConfirm(
                title = activity.getString(CoreR.string.unsupport_nonroot_stub_title),
                content = activity.getString(CoreR.string.unsupport_nonroot_stub_msg),
                confirm = activity.getString(CoreR.string.install),
            )
        }
    }

    for ((index, pair) in unsupportedMessages.withIndex()) {
        val (titleRes, msgRes) = pair
        val show = rememberSaveable { androidx.compose.runtime.mutableStateOf(true) }
        com.topjohnwu.magisk.ui.component.rememberConfirmDialog(
            onConfirm = { show.value = false },
        ).also { dialog ->
            LaunchedEffect(Unit) {
                dialog.showConfirm(
                    title = activity.getString(titleRes),
                    content = activity.getString(msgRes),
                )
            }
        }
    }

    val shortcutDialog = com.topjohnwu.magisk.ui.component.rememberConfirmDialog(
        onConfirm = {
            activity.showShortcutPrompt.value = false
            Shortcuts.addHomeIcon(activity)
        },
        onDismiss = { activity.showShortcutPrompt.value = false }
    )

    LaunchedEffect(showShortcut) {
        if (showShortcut) {
            shortcutDialog.showConfirm(
                title = activity.getString(CoreR.string.add_shortcut_title),
                content = activity.getString(CoreR.string.add_shortcut_msg),
            )
        }
    }
}
