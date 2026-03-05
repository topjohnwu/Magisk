package com.topjohnwu.magisk.ui.theme

import android.os.Build
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.platform.LocalContext
import com.topjohnwu.magisk.core.Config

@Composable
fun magiskComposeColorScheme(
    useDynamicColor: Boolean,
    darkTheme: Boolean,
    selectedTheme: Theme = Theme.selected
): ColorScheme {
    val scheme = if (selectedTheme == Theme.Default) {
        when {
            useDynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
                if (darkTheme) dynamicDarkColorScheme(LocalContext.current)
                else dynamicLightColorScheme(LocalContext.current)
            }

            else -> defaultFallbackScheme(darkTheme)
        }
    } else {
        composeLegacyThemeScheme(selectedTheme, darkTheme)
    }

    val forceAmoledBase = darkTheme && Config.darkTheme == Config.Value.DARK_THEME_AMOLED
    return if (forceAmoledBase) scheme.toAmoledBaseScheme() else scheme
}

@Composable
private fun composeLegacyThemeScheme(theme: Theme, darkTheme: Boolean): ColorScheme {
    val seed = when (theme) {
        Theme.Piplup -> PIPLUP
        Theme.PiplupAmoled -> PIPLUP
        Theme.Rayquaza -> RAYQUAZA
        Theme.Zapdos -> ZAPDOS
        Theme.Charmeleon -> CHARMELEON
        Theme.Mew -> MEW
        Theme.Salamence -> SALAMENCE
        Theme.Fraxure -> FRAXURE
        Theme.Custom -> customSeed()
        Theme.Default -> null
    } ?: return defaultFallbackScheme(darkTheme)
    return seed.toColorScheme(darkTheme)
}

private fun customSeed(): ComposeThemeSeed = ComposeThemeSeed(
    lightPrimary = Color(Config.themeCustomLightPrimary),
    darkPrimary = Color(Config.themeCustomDarkPrimary),
    lightSecondary = Color(Config.themeCustomLightSecondary),
    darkSecondary = Color(Config.themeCustomDarkSecondary),
    lightSurface = Color(Config.themeCustomLightSurface),
    darkSurface = Color(Config.themeCustomDarkSurface),
    lightOnSurface = Color(Config.themeCustomLightOnSurface),
    darkOnSurface = Color(Config.themeCustomDarkOnSurface),
    lightError = Color(Config.themeCustomLightError),
    darkError = Color(Config.themeCustomDarkError)
)

private data class ComposeThemeSeed(
    val lightPrimary: Color,
    val darkPrimary: Color,
    val lightSecondary: Color,
    val darkSecondary: Color,
    val lightSurface: Color,
    val darkSurface: Color,
    val lightOnSurface: Color,
    val darkOnSurface: Color,
    val lightError: Color,
    val darkError: Color
)

private fun ComposeThemeSeed.toColorScheme(darkTheme: Boolean): ColorScheme {
    val primary = if (darkTheme) darkPrimary else lightPrimary
    val secondary = if (darkTheme) darkSecondary else lightSecondary
    val surface = if (darkTheme) darkSurface else lightSurface
    val onSurface = if (darkTheme) darkOnSurface else lightOnSurface
    val error = if (darkTheme) darkError else lightError

    val blendTarget = if (darkTheme) BLACK else WHITE
    val variantTarget = if (darkTheme) WHITE else BLACK
    val tertiary = blend(primary, secondary, 0.42f)

    val primaryContainer = blend(primary, blendTarget, if (darkTheme) 0.42f else 0.78f)
    val secondaryContainer = blend(secondary, blendTarget, if (darkTheme) 0.40f else 0.76f)
    val tertiaryContainer = blend(tertiary, blendTarget, if (darkTheme) 0.40f else 0.76f)
    val errorContainer = blend(error, blendTarget, if (darkTheme) 0.40f else 0.78f)
    val surfaceVariant = blend(surface, variantTarget, if (darkTheme) 0.08f else 0.06f)

    val outline = blend(onSurface, surface, 0.58f)
    val outlineVariant = blend(onSurface, surface, 0.74f)
    val inverseSurface =
        blend(surface, if (darkTheme) WHITE else BLACK, if (darkTheme) 0.86f else 0.80f)
    val inversePrimary =
        blend(primary, if (darkTheme) WHITE else BLACK, if (darkTheme) 0.34f else 0.22f)

    return if (darkTheme) {
        darkColorScheme(
            primary = primary,
            onPrimary = contentColorFor(primary),
            primaryContainer = primaryContainer,
            onPrimaryContainer = contentColorFor(primaryContainer),
            secondary = secondary,
            onSecondary = contentColorFor(secondary),
            secondaryContainer = secondaryContainer,
            onSecondaryContainer = contentColorFor(secondaryContainer),
            tertiary = tertiary,
            onTertiary = contentColorFor(tertiary),
            tertiaryContainer = tertiaryContainer,
            onTertiaryContainer = contentColorFor(tertiaryContainer),
            error = error,
            onError = contentColorFor(error),
            errorContainer = errorContainer,
            onErrorContainer = contentColorFor(errorContainer),
            background = surface,
            onBackground = onSurface,
            surface = surface,
            onSurface = onSurface,
            surfaceVariant = surfaceVariant,
            onSurfaceVariant = blend(onSurface, BLACK, 0.16f),
            outline = outline,
            outlineVariant = outlineVariant,
            inverseSurface = inverseSurface,
            inverseOnSurface = contentColorFor(inverseSurface),
            inversePrimary = inversePrimary,
            surfaceTint = primary,
            scrim = BLACK
        )
    } else {
        lightColorScheme(
            primary = primary,
            onPrimary = contentColorFor(primary),
            primaryContainer = primaryContainer,
            onPrimaryContainer = contentColorFor(primaryContainer),
            secondary = secondary,
            onSecondary = contentColorFor(secondary),
            secondaryContainer = secondaryContainer,
            onSecondaryContainer = contentColorFor(secondaryContainer),
            tertiary = tertiary,
            onTertiary = contentColorFor(tertiary),
            tertiaryContainer = tertiaryContainer,
            onTertiaryContainer = contentColorFor(tertiaryContainer),
            error = error,
            onError = contentColorFor(error),
            errorContainer = errorContainer,
            onErrorContainer = contentColorFor(errorContainer),
            background = surface,
            onBackground = onSurface,
            surface = surface,
            onSurface = onSurface,
            surfaceVariant = surfaceVariant,
            onSurfaceVariant = blend(onSurface, WHITE, 0.22f),
            outline = outline,
            outlineVariant = outlineVariant,
            inverseSurface = inverseSurface,
            inverseOnSurface = contentColorFor(inverseSurface),
            inversePrimary = inversePrimary,
            surfaceTint = primary,
            scrim = BLACK
        )
    }
}

private fun contentColorFor(color: Color): Color {
    return if (color.luminance() > 0.42f) BLACK else WHITE
}

private fun blend(base: Color, overlay: Color, amount: Float): Color {
    val safeAmount = amount.coerceIn(0f, 1f)
    val inv = 1f - safeAmount
    return Color(
        red = base.red * inv + overlay.red * safeAmount,
        green = base.green * inv + overlay.green * safeAmount,
        blue = base.blue * inv + overlay.blue * safeAmount,
        alpha = 1f
    )
}

private fun defaultFallbackScheme(darkTheme: Boolean): ColorScheme = PIPLUP.toColorScheme(darkTheme)

private fun ColorScheme.toAmoledBaseScheme(): ColorScheme = copy(
    background = BLACK,
    surface = BLACK,
    surfaceVariant = Color(0xFF0D0D0D),
    surfaceDim = BLACK,
    surfaceBright = Color(0xFF171717),
    surfaceContainerLowest = BLACK,
    surfaceContainerLow = Color(0xFF050505),
    surfaceContainer = Color(0xFF090909),
    surfaceContainerHigh = Color(0xFF0E0E0E),
    surfaceContainerHighest = Color(0xFF141414),
    inverseSurface = Color(0xFFE8E8E8),
    inverseOnSurface = Color(0xFF141414),
    scrim = BLACK
)

private val WHITE = Color(0xFFFFFFFF)
private val BLACK = Color(0xFF000000)

private val PIPLUP = ComposeThemeSeed(
    lightPrimary = Color(0xFF4EAFF5),
    darkPrimary = Color(0xFF4EAFF5),
    lightSecondary = Color(0xFF3E78AF),
    darkSecondary = Color(0xFF3E78AF),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val RAYQUAZA = ComposeThemeSeed(
    lightPrimary = Color(0xFF68A17F),
    darkPrimary = Color(0xFF68A17F),
    lightSecondary = Color(0xFF2F6D43),
    darkSecondary = Color(0xFF2F6D43),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val ZAPDOS = ComposeThemeSeed(
    lightPrimary = Color(0xFFF2B90D),
    darkPrimary = Color(0xFFFBD179),
    lightSecondary = Color(0xFFB29667),
    darkSecondary = Color(0xFFB29667),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val CHARMELEON = ComposeThemeSeed(
    lightPrimary = Color(0xFFDB7366),
    darkPrimary = Color(0xFFDB7366),
    lightSecondary = Color(0xFFB65247),
    darkSecondary = Color(0xFFB65247),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val MEW = ComposeThemeSeed(
    lightPrimary = Color(0xFFB3566C),
    darkPrimary = Color(0xFFD9ADB7),
    lightSecondary = Color(0xFFB5889B),
    darkSecondary = Color(0xFFB5889B),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val SALAMENCE = ComposeThemeSeed(
    lightPrimary = Color(0xFF70B2C6),
    darkPrimary = Color(0xFF70B2C6),
    lightSecondary = Color(0xFFC06A75),
    darkSecondary = Color(0xFFC06A75),
    lightSurface = Color(0xFFF9F9F9),
    darkSurface = Color(0xFF0D0D0D),
    lightOnSurface = Color(0xFF444444),
    darkOnSurface = Color(0xFFD8D8D8),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)

private val FRAXURE = ComposeThemeSeed(
    lightPrimary = Color(0xFF009688),
    darkPrimary = Color(0xFF26B7AA),
    lightSecondary = Color(0xFFCC0047),
    darkSecondary = Color(0xFFEF8282),
    lightSurface = Color(0xFFFFFFFF),
    darkSurface = Color(0xFF000000),
    lightOnSurface = Color(0xFF242424),
    darkOnSurface = Color(0xFFEDEDED),
    lightError = Color(0xFFCC0047),
    darkError = Color(0xFFEF8282)
)
