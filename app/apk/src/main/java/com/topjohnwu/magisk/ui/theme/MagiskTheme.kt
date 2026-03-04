package com.topjohnwu.magisk.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.setValue
import com.topjohnwu.magisk.core.Config
import top.yukonga.miuix.kmp.theme.ColorSchemeMode
import top.yukonga.miuix.kmp.theme.LocalContentColor
import top.yukonga.miuix.kmp.theme.MiuixTheme
import top.yukonga.miuix.kmp.theme.ThemeController

object ThemeState {
    var colorMode by mutableIntStateOf(Config.colorMode)
}

@Composable
fun MagiskTheme(
    content: @Composable () -> Unit
) {
    val isDark = isSystemInDarkTheme()
    val mode = ThemeState.colorMode
    val controller = when (mode) {
        1 -> ThemeController(ColorSchemeMode.Light)
        2 -> ThemeController(ColorSchemeMode.Dark)
        3 -> ThemeController(ColorSchemeMode.MonetSystem, isDark = isDark)
        4 -> ThemeController(ColorSchemeMode.MonetLight)
        5 -> ThemeController(ColorSchemeMode.MonetDark)
        else -> ThemeController(ColorSchemeMode.System)
    }
    MiuixTheme(controller = controller) {
        CompositionLocalProvider(
            LocalContentColor provides MiuixTheme.colorScheme.onBackground,
            content = content
        )
    }
}
