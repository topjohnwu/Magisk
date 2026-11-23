package com.topjohnwu.magisk.wear

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.res.Configuration
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalResources
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.core.net.toUri
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.viewmodel.navigation3.rememberViewModelStoreNavEntryDecorator
import androidx.navigation3.runtime.entryProvider
import androidx.navigation3.runtime.rememberSaveableStateHolderNavEntryDecorator
import androidx.navigation3.ui.NavDisplay
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.AppScaffold
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import androidx.wear.compose.material3.TimeText
import androidx.wear.compose.navigation3.rememberSwipeDismissableSceneStrategy
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.ActivityExtension
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.flash.queryDisplayName
import com.topjohnwu.magisk.ui.flash.sanitizeDisplayName
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.superuser.SuperuserGrantViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.ui.unsupportedMessages
import com.topjohnwu.magisk.wear.files.PickScreen
import com.topjohnwu.magisk.wear.home.HomeScreen
import com.topjohnwu.magisk.wear.nav.LocalWearNavigator
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import com.topjohnwu.magisk.wear.nav.rememberWearNavigator
import com.topjohnwu.magisk.wear.superuser.GrantScreen
import com.topjohnwu.magisk.wear.superuser.SuperuserDetailScreen
import com.topjohnwu.magisk.wear.superuser.SuperuserScreen
import com.topjohnwu.magisk.wear.theme.WearTheme
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.flow.MutableStateFlow
import com.topjohnwu.magisk.core.R as CoreR

/**
 * Whether this device gets the Wear OS UI: a watch on API 25 or later, which Wear Compose
 * requires. The check matches the `watch-v25` resource qualifier, so the theme and splash
 * colors in `values-watch-v25` apply exactly where this returns true.
 */
fun Context.useWearUi(): Boolean =
    Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1 &&
        (resources.configuration.uiMode and Configuration.UI_MODE_TYPE_MASK) ==
        Configuration.UI_MODE_TYPE_WATCH

/**
 * The Wear OS UI of the app. On a device where [useWearUi] is true, the main activity hands
 * its content and new intents to this class instead of showing the phone UI. The activity
 * keeps doing the common splash and initialization sequence.
 */
class WearUi(
    private val activity: ComponentActivity,
    private val extension: ActivityExtension,
) {

    private val intentState = MutableStateFlow(0)

    @SuppressLint("InlinedApi")
    fun onCreateUi(savedInstanceState: Bundle?) {
        val unsupported = activity.unsupportedMessages()

        // Only ask on startup, not every time the activity is recreated
        if (savedInstanceState == null && Config.checkUpdate) {
            extension.withPermission(Manifest.permission.POST_NOTIFICATIONS) {
                Config.checkUpdate = it
            }
        }

        activity.setContent {
            WearTheme {
                AppScaffold(timeText = { TimeText() }) {
                    val navigator = rememberWearNavigator(Route.Main)
                    CompositionLocalProvider(LocalWearNavigator provides navigator) {
                        HandleFlashIntent(navigator)

                        NavDisplay(
                            backStack = navigator.backStack,
                            onBack = { navigator.pop() },
                            entryDecorators = listOf(
                                rememberSaveableStateHolderNavEntryDecorator(),
                                rememberViewModelStoreNavEntryDecorator<Any>()
                            ),
                            // Swipe-to-dismiss, and predictive back from API 36
                            sceneStrategies = listOf(rememberSwipeDismissableSceneStrategy()),
                            entryProvider = entryProvider {
                                entry<Route.Main> { HomeScreen(navigator) }
                                entry<Route.DenyList> {
                                    Placeholder(stringResource(CoreR.string.denylist))
                                }
                                entry<Route.Flash> {
                                    Placeholder(stringResource(CoreR.string.flash_screen_title))
                                }
                                entry<Route.SuperuserDetail> { key ->
                                    // Owned by the activity, so the list and its details share one
                                    // copy of the policies, like the phone app
                                    val vm: SuperuserViewModel = viewModel(
                                        viewModelStoreOwner = activity, factory = VMFactory
                                    )
                                    SuperuserDetailScreen(key.uid, vm, navigator)
                                }
                                entry<Route.SuperuserGrant> {
                                    val vm: SuperuserGrantViewModel = viewModel(factory = VMFactory)
                                    LaunchedEffect(Unit) {
                                        vm.startLoading()
                                    }
                                    GrantScreen(vm)
                                }
                                entry<Route.Action> { key -> Placeholder(key.name) }
                                entry<WearRoute.Superuser> {
                                    val vm: SuperuserViewModel = viewModel(
                                        viewModelStoreOwner = activity, factory = VMFactory
                                    )
                                    SuperuserScreen(vm, navigator)
                                }
                                entry<WearRoute.Logs> {
                                    Placeholder(stringResource(CoreR.string.logs))
                                }
                                entry<WearRoute.Modules> {
                                    Placeholder(stringResource(CoreR.string.modules))
                                }
                                entry<WearRoute.ModuleDetail> { key -> Placeholder(key.dir) }
                                entry<WearRoute.Install> {
                                    Placeholder(stringResource(CoreR.string.install))
                                }
                                entry<WearRoute.Settings> {
                                    Placeholder(stringResource(CoreR.string.settings))
                                }
                                entry<WearRoute.Pick> { key -> PickScreen(key.kind, navigator) }
                                entry<WearRoute.Restore> {
                                    Placeholder(stringResource(CoreR.string.restore_img))
                                }
                            }
                        )
                    }
                    UnsupportedDialogs(unsupported)
                }
            }
        }
    }

    @Composable
    private fun HandleFlashIntent(navigator: WearNavigator) {
        val context = LocalContext.current
        val resources = LocalResources.current
        val intentVersion by intentState.collectAsStateWithLifecycle()
        var confirmRequest by remember { mutableStateOf<ConfirmRequest?>(null) }
        // Not cleared, so the text stays while the dialog animates out after it is answered
        var shownContent by remember { mutableStateOf("") }

        LaunchedEffect(intentVersion) {
            val currentIntent = activity.intent ?: return@LaunchedEffect
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
                        val confirmed = try {
                            val request = ConfirmRequest(
                                content = resources.getString(CoreR.string.confirm_install, displayName)
                            )
                            shownContent = request.content
                            confirmRequest = request
                            request.result.await()
                        } finally {
                            confirmRequest = null
                            setHideOverlayWindows(false)
                        }
                        if (confirmed) Route.Flash(action, uriString) else null
                    }
                }
                // Only installing modules can be requested through this intent (our module
                // downloads use it too); drop any other action, e.g. uninstalling Magisk
                else -> null
            }

            // Only consume the intent once the request is resolved. If a configuration change
            // cancels this effect while the confirmation is showing, the new Activity replays it.
            currentIntent.action = null
            route?.let { navigator.navigate(it) }
        }

        val request = confirmRequest
        AlertDialog(
            visible = request != null,
            onDismissRequest = { request?.result?.complete(false) },
            title = { Text(stringResource(CoreR.string.confirm_install_title)) },
            text = { Text(shownContent) },
            confirmButton = {
                AlertDialogDefaults.ConfirmButton(onClick = { request?.result?.complete(true) })
            },
            dismissButton = {
                AlertDialogDefaults.DismissButton(onClick = { request?.result?.complete(false) })
            },
        )
    }

    /** A confirmation shown by [HandleFlashIntent], completed with the user's answer. */
    private class ConfirmRequest(val content: String) {
        val result = CompletableDeferred<Boolean>()
    }

    private fun setHideOverlayWindows(hide: Boolean) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            activity.window.setHideOverlayWindows(hide)
        }
    }

    /** Called by the activity after it got a new intent. */
    fun onNewIntent() {
        intentState.value += 1
    }
}

/** Stands in for a screen that is not implemented yet. */
@Composable
private fun Placeholder(title: String) {
    ScreenScaffold { contentPadding ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(contentPadding),
            contentAlignment = Alignment.Center,
        ) {
            Text(title, textAlign = TextAlign.Center)
        }
    }
}

/** Shows each problem found by [unsupportedMessages] in turn, like the phone app on startup. */
@Composable
private fun UnsupportedDialogs(messages: List<Pair<Int, Int>>) {
    var index by rememberSaveable { mutableIntStateOf(0) }
    // The last one stays as content while its dialog animates out
    val shown = messages.getOrNull(index) ?: messages.lastOrNull() ?: return
    AlertDialog(
        visible = index < messages.size,
        onDismissRequest = { index++ },
        title = { Text(stringResource(shown.first)) },
        text = { Text(stringResource(shown.second)) },
        edgeButton = { AlertDialogDefaults.EdgeButton(onClick = { index++ }) },
    )
}
