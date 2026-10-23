package com.topjohnwu.magisk.ui.navigation

import androidx.compose.runtime.Composable
import androidx.compose.runtime.saveable.Saver
import androidx.compose.runtime.saveable.listSaver
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.navigation3.runtime.NavBackStack
import androidx.navigation3.runtime.NavKey

class Navigator(val backStack: NavBackStack<NavKey>) {
    constructor(vararg initialKeys: NavKey) : this(NavBackStack(*initialKeys))

    fun push(key: NavKey) {
        backStack.add(key)
    }

    fun pop(): Boolean {
        // Never remove the root: NavDisplay requires a non-empty back stack, and a screen's
        // back button stays clickable during its exit animation, so it can be tapped again
        if (backStack.size <= 1) return false
        backStack.removeAt(backStack.lastIndex)
        return true
    }

    fun current(): NavKey? = backStack.lastOrNull()

    companion object {
        val Saver: Saver<Navigator, Any> = listSaver(
            save = { navigator -> navigator.backStack.toList() },
            restore = { savedList ->
                val stack = NavBackStack<NavKey>()
                if (savedList.isNotEmpty()) {
                    stack.addAll(savedList)
                } else {
                    stack.add(Route.Main)
                }
                Navigator(stack)
            }
        )
    }
}

@Composable
fun rememberNavigator(vararg startRoutes: NavKey = arrayOf(Route.Main)): Navigator {
    return rememberSaveable(saver = Navigator.Saver) {
        Navigator(*startRoutes)
    }
}

val LocalNavigator = staticCompositionLocalOf<Navigator> {
    error("LocalNavigator not provided")
}
