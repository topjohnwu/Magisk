package com.topjohnwu.magisk.wear.nav

import android.net.Uri
import android.os.Parcelable
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.saveable.Saver
import androidx.compose.runtime.saveable.listSaver
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.navigation3.runtime.NavBackStack
import androidx.navigation3.runtime.NavKey
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.wear.files.FileKind
import kotlinx.parcelize.Parcelize

/**
 * Screens that only exist on Wear. Screens shared with the phone app use [Route], so view
 * models in :shared can navigate to them with [BaseViewModel.navigateTo].
 */
sealed interface WearRoute : NavKey, Parcelable {
    @Parcelize
    data object Superuser : WearRoute

    @Parcelize
    data object Logs : WearRoute

    @Parcelize
    data object Modules : WearRoute

    @Parcelize
    data class ModuleDetail(
        // The module's directory name: unlike the id in module.prop, it is unique
        val dir: String,
    ) : WearRoute

    @Parcelize
    data object Install : WearRoute

    @Parcelize
    data object Settings : WearRoute

    /** The root shell file list, used when no app handles ACTION_OPEN_DOCUMENT. */
    @Parcelize
    data class Pick(val kind: FileKind) : WearRoute

    @Parcelize
    data object Restore : WearRoute
}

class WearNavigator(val backStack: NavBackStack<NavKey>) {
    constructor(vararg initialKeys: NavKey) : this(NavBackStack(*initialKeys))

    // Files picked on a WearRoute.Pick screen, waiting for the picker that opened it.
    // See setPickResult for how a result reaches its caller.
    private val pickResults = mutableStateMapOf<FileKind, Uri>()

    fun navigate(key: NavKey) {
        // Ignore pushing the screen that is already on top (e.g. a double tap, or the same
        // request delivered twice). Both copies would get the same content key, so they would
        // share one ViewModel and saved state, and need two swipes to leave.
        if (backStack.lastOrNull() == key) return
        // A result nobody consumed belongs to an earlier pick, never to this one
        if (key is WearRoute.Pick) pickResults.remove(key.kind)
        backStack.add(key)
    }

    fun pop(): Boolean {
        // Never remove the root: NavDisplay requires a non-empty back stack. Swiping the root
        // away is left to the system, which closes the app.
        if (backStack.size <= 1) return false
        backStack.removeAt(backStack.lastIndex)
        return true
    }

    fun current(): NavKey? = backStack.lastOrNull()

    /**
     * Hand a file picked on the [WearRoute.Pick] screen of [kind] to its caller.
     *
     * The Pick screen sets the result and pops itself. The caller is the screen right below
     * it, so the rememberFilePicker of the same kind on that screen is composed again and
     * takes the result with [consumePickResult]. A result is kept for one kind at a time,
     * is dropped when a new Pick of that kind is opened, and is not saved across process
     * death (it is consumed as soon as the caller is shown, so nothing is pending by then).
     */
    fun setPickResult(kind: FileKind, uri: Uri) {
        pickResults[kind] = uri
    }

    /** The result waiting for [kind], read as snapshot state so callers recompose on it. */
    fun pickResult(kind: FileKind): Uri? = pickResults[kind]

    fun consumePickResult(kind: FileKind): Uri? = pickResults.remove(kind)

    companion object {
        val Saver: Saver<WearNavigator, Any> = listSaver(
            save = { navigator -> navigator.backStack.toList() },
            restore = { savedList ->
                val stack = NavBackStack<NavKey>()
                if (savedList.isNotEmpty()) {
                    stack.addAll(savedList)
                } else {
                    stack.add(Route.Main)
                }
                WearNavigator(stack)
            }
        )
    }
}

@Composable
fun rememberWearNavigator(vararg startRoutes: NavKey = arrayOf(Route.Main)): WearNavigator {
    return rememberSaveable(saver = WearNavigator.Saver) {
        WearNavigator(*startRoutes)
    }
}

/** For composables that navigate without being a screen, e.g. the file picker. */
val LocalWearNavigator = staticCompositionLocalOf<WearNavigator> {
    error("LocalWearNavigator not provided")
}

@Composable
fun CollectNavEvents(viewModel: BaseViewModel, navigator: WearNavigator) {
    LaunchedEffect(viewModel) {
        viewModel.navEvents.collect { route ->
            navigator.navigate(route)
        }
    }
}
