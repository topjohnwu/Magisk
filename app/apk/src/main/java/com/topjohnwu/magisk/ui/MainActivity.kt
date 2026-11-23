package com.topjohnwu.magisk.ui

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.animation.AnimatedContentTransitionScope.SlideDirection
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.viewmodel.navigation3.rememberViewModelStoreNavEntryDecorator
import androidx.navigation3.runtime.entryProvider
import androidx.navigation3.runtime.rememberSaveableStateHolderNavEntryDecorator
import androidx.navigation3.ui.NavDisplay
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.ActivityExtension
import com.topjohnwu.magisk.core.base.SplashController
import com.topjohnwu.magisk.core.base.SplashScreenHost
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ui.component.ConfirmResult
import com.topjohnwu.magisk.ui.component.rememberConfirmDialog
import com.topjohnwu.magisk.ui.deny.DenyListScreen
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.ui.flash.FlashScreen
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.flash.queryDisplayName
import com.topjohnwu.magisk.ui.flash.sanitizeDisplayName
import com.topjohnwu.magisk.ui.module.ActionScreen
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Navigator
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.navigation.rememberNavigator
import com.topjohnwu.magisk.ui.superuser.SuperuserDetailScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserGrantScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserGrantViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.wear.WearUi
import com.topjohnwu.magisk.wear.useWearUi
import kotlinx.coroutines.flow.MutableStateFlow
import com.topjohnwu.magisk.core.R as CoreR

class MainActivity : ComponentActivity(), SplashScreenHost {

    override val extension = ActivityExtension(this)
    override val splashController = SplashController(this)

    private val intentState = MutableStateFlow(0)
    // A tab requested by a new intent while the activity is running (it's singleTask)
    private val tabRequest = MutableStateFlow<Int?>(null)
    internal val showUnsupported = MutableStateFlow<List<Pair<Int, Int>>>(emptyList())
    // Set in onCreate where the Wear OS UI replaces this one
    private var wearUi: WearUi? = null

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        if (useWearUi()) wearUi = WearUi(this, extension) else applySystemBarStyle()
        extension.onCreate(savedInstanceState)
        splashController.preOnCreate()
        super.onCreate(savedInstanceState)
        splashController.onCreate(savedInstanceState)
    }

    override fun onApplyThemeResource(theme: Resources.Theme, resid: Int, first: Boolean) {
        super.onApplyThemeResource(theme, resid, first)
        theme.applyStyle(R.style.Main, true)
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
        wearUi?.let {
            it.onCreateUi(savedInstanceState)
            return
        }

        showUnsupportedMessage()

        // Only ask on startup, not every time the activity is recreated
        if (savedInstanceState == null && Config.checkUpdate) {
            extension.withPermission(Manifest.permission.POST_NOTIFICATIONS) {
                Config.checkUpdate = it
            }
        }

        val initialTab = getRequestedTab(intent) ?: Tab.HOME.ordinal

        setContent {
            MagiskTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
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
                            // Start/End follow the layout direction, so RTL slides the other way
                            transitionSpec = {
                                slideIntoContainer(SlideDirection.Start) togetherWith
                                    slideOutOfContainer(SlideDirection.Start)
                            },
                            popTransitionSpec = {
                                slideIntoContainer(SlideDirection.End) togetherWith
                                    slideOutOfContainer(SlideDirection.End)
                            },
                            predictivePopTransitionSpec = {
                                slideIntoContainer(SlideDirection.End) togetherWith
                                    slideOutOfContainer(SlideDirection.End)
                            },
                            entryProvider = entryProvider {
                                entry<Route.Main> {
                                    val superuserVm: SuperuserViewModel = viewModel(
                                        viewModelStoreOwner = this@MainActivity, factory = VMFactory
                                    )
                                    val requestedTab by tabRequest.collectAsStateWithLifecycle()
                                    MainScreen(
                                        initialTab = initialTab,
                                        requestedTab = requestedTab,
                                        onTabRequestHandled = { tabRequest.value = null },
                                        superuserViewModel = superuserVm,
                                    )
                                }
                                entry<Route.DenyList> { _ ->
                                    val vm: DenyListViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(Unit) { vm.startLoading() }
                                    DenyListScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.Flash> { key ->
                                    val vm: FlashViewModel = viewModel(factory = VMFactory)
                                    if (vm.isInterrupted) {
                                        // Restored after process death: nothing is left to show
                                        LeaveInterruptedScreen(navigator, key)
                                        return@entry
                                    }
                                    LaunchedEffect(key) {
                                        if (vm.flashAction.isEmpty()) {
                                            vm.flashAction = key.action
                                            vm.flashUri = key.additionalData?.toUri()
                                            vm.usbDevice = key.usbDevice
                                            vm.startFlashing()
                                        }
                                    }
                                    FlashScreen(vm, onBack = { navigator.pop() })
                                }
                                entry<Route.SuperuserDetail> { key ->
                                    val vm: SuperuserViewModel = viewModel(
                                        viewModelStoreOwner = this@MainActivity, factory = VMFactory
                                    )
                                    SuperuserDetailScreen(
                                        uid = key.uid,
                                        viewModel = vm,
                                        onBack = { navigator.pop() },
                                    )
                                }
                                entry<Route.SuperuserGrant> { _ ->
                                    val vm: SuperuserGrantViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(Unit) {
                                        vm.startLoading()
                                    }
                                    SuperuserGrantScreen(
                                        viewModel = vm,
                                        onBack = { navigator.pop() },
                                    )
                                }
                                entry<Route.Action> { key ->
                                    val vm: ActionViewModel = viewModel(factory = VMFactory)
                                    if (vm.isInterrupted) {
                                        // Restored after process death: nothing is left to show
                                        LeaveInterruptedScreen(navigator, key)
                                        return@entry
                                    }
                                    LaunchedEffect(key) {
                                        if (vm.actionDir.isEmpty()) {
                                            vm.actionDir = key.dir
                                            vm.actionName = key.name
                                            vm.startRunAction()
                                        }
                                    }
                                    ActionScreen(vm, actionName = key.name, onBack = { navigator.pop() })
                                }
                            }
                        )
                    }
                    MainActivityDialogs(
                        unsupportedMessages = showUnsupported.collectAsStateWithLifecycle().value,
                    )
                }
            }
        }
    }

    @Composable
    private fun HandleFlashIntent(navigator: Navigator) {
        val confirmDialog = rememberConfirmDialog()
        val context = LocalContext.current
        val resources = LocalResources.current
        val intentVersion by intentState.collectAsStateWithLifecycle()

        LaunchedEffect(intentVersion) {
            val currentIntent = intent ?: return@LaunchedEffect
            if (currentIntent.action != FlashUtils.INTENT_FLASH) return@LaunchedEffect
            val action = currentIntent.getStringExtra(FlashUtils.EXTRA_FLASH_ACTION)
            val uriString = currentIntent.getStringExtra(FlashUtils.EXTRA_FLASH_URI)

            val route = when (action) {
                Const.Value.FLASH_ZIP -> {
                    if (uriString.isNullOrEmpty()) {
                        null
                    } else if (FlashUtils.isTrusted(currentIntent)) {
                        // Sent by our own module download, which the user already chose to install
                        Route.Flash(action, uriString)
                    } else {
                        val uri = uriString.toUri()
                        val name = queryDisplayName(context, uri)?.sanitizeDisplayName()?.ifEmpty { null }
                            ?: uri.lastPathSegment?.sanitizeDisplayName()?.ifEmpty { null }
                            ?: "module.zip"
                        // The name is chosen by the app that sent the intent, so also show where it comes from
                        val authority = uri.authority?.sanitizeDisplayName()?.ifEmpty { null }
                        val displayName = authority?.let { "$name ($it)" } ?: name

                        // This confirmation is the only safeguard, so don't let other apps draw over it
                        setHideOverlayWindows(true)
                        val result = try {
                            confirmDialog.awaitConfirm(
                                title = resources.getString(CoreR.string.confirm_install_title),
                                content = resources.getString(CoreR.string.confirm_install, displayName)
                            )
                        } finally {
                            setHideOverlayWindows(false)
                        }
                        if (result == ConfirmResult.Confirmed) Route.Flash(action, uriString) else null
                    }
                }
                // Only installing modules can be requested through this intent (our module
                // downloads use it too); drop any other action, e.g. uninstalling Magisk
                else -> null
            }

            // Only consume the intent once the request is resolved. If a configuration change
            // cancels this effect while the confirmation is showing, the new Activity replays it.
            currentIntent.action = null
            route?.let { navigator.push(it) }
        }
    }

    private fun setHideOverlayWindows(hide: Boolean) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.setHideOverlayWindows(hide)
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        wearUi?.let {
            it.onNewIntent()
            return
        }
        getRequestedTab(intent)?.let { tabRequest.value = it }
        intentState.value += 1
    }

    private fun getRequestedTab(intent: Intent?): Int? {
        val section = when {
            intent?.action == Intent.ACTION_APPLICATION_PREFERENCES -> Const.Nav.SETTINGS
            intent?.action == FlashUtils.INTENT_FLASH &&
                intent.getStringExtra(FlashUtils.EXTRA_FLASH_ACTION) == Const.Value.FLASH_ZIP -> Const.Nav.MODULES
            else -> intent?.getStringExtra(Const.Key.OPEN_SECTION)
        }
        return when (section) {
            Const.Nav.HOME -> Tab.HOME.ordinal
            Const.Nav.SUPERUSER -> Tab.SUPERUSER.ordinal
            Const.Nav.MODULES -> Tab.MODULES.ordinal
            Const.Nav.SETTINGS -> Tab.SETTINGS.ordinal
            else -> null
        }
    }

    private fun showUnsupportedMessage() {
        val messages = unsupportedMessages()
        if (messages.isNotEmpty()) {
            showUnsupported.value = messages
        }
    }
}

/**
 * Leave a flash or action screen restored after process death. Its job and output only lived
 * in memory, and running the job again may not be safe, so there is nothing left to show.
 */
@Composable
private fun LeaveInterruptedScreen(navigator: Navigator, key: Route) {
    LaunchedEffect(key) {
        if (navigator.current() == key) navigator.pop()
    }
}

@Composable
private fun MainActivityDialogs(
    unsupportedMessages: List<Pair<Int, Int>>,
    modifier: Modifier = Modifier
) {
    val resources = LocalResources.current

    var currentUnsupportedIndex by rememberSaveable { mutableIntStateOf(0) }
    val unsupportedDialog = rememberConfirmDialog(
        onConfirm = { currentUnsupportedIndex++ },
        onDismiss = { currentUnsupportedIndex++ }
    )

    val currentUnsupported = unsupportedMessages.getOrNull(currentUnsupportedIndex)
    LaunchedEffect(currentUnsupported) {
        if (currentUnsupported != null) {
            val (titleRes, msgRes) = currentUnsupported
            unsupportedDialog.showConfirm(
                title = resources.getString(titleRes),
                content = resources.getString(msgRes),
            )
        }
    }
}

