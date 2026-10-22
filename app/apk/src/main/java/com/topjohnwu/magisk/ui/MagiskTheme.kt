package com.topjohnwu.magisk.ui

import android.annotation.SuppressLint
import android.content.res.Configuration
import android.content.res.Resources
import android.os.Build
import androidx.activity.ComponentActivity
import androidx.activity.SystemBarStyle
import androidx.activity.compose.LocalActivity
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Shapes
import androidx.compose.material3.Typography
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.google.android.material.color.utilities.DynamicColor
import com.google.android.material.color.utilities.Hct
import com.google.android.material.color.utilities.MaterialDynamicColors
import com.google.android.material.color.utilities.SchemeTonalSpot
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.model.ColorMode

@SuppressLint("RestrictedApi")
fun dynamicColorScheme(
    seed: Color,
    isDark: Boolean,
    contrastLevel: Double = 0.0
): ColorScheme {
    val scheme = SchemeTonalSpot(Hct.fromInt(seed.toArgb()), isDark, contrastLevel)
    val dyn = MaterialDynamicColors()
    fun DynamicColor.toColor() = Color(getArgb(scheme))

    return ColorScheme(
        primary = dyn.primary().toColor(),
        onPrimary = dyn.onPrimary().toColor(),
        primaryContainer = dyn.primaryContainer().toColor(),
        onPrimaryContainer = dyn.onPrimaryContainer().toColor(),
        inversePrimary = dyn.inversePrimary().toColor(),
        secondary = dyn.secondary().toColor(),
        onSecondary = dyn.onSecondary().toColor(),
        secondaryContainer = dyn.secondaryContainer().toColor(),
        onSecondaryContainer = dyn.onSecondaryContainer().toColor(),
        tertiary = dyn.tertiary().toColor(),
        onTertiary = dyn.onTertiary().toColor(),
        tertiaryContainer = dyn.tertiaryContainer().toColor(),
        onTertiaryContainer = dyn.onTertiaryContainer().toColor(),
        background = dyn.background().toColor(),
        onBackground = dyn.onBackground().toColor(),
        surface = dyn.surface().toColor(),
        onSurface = dyn.onSurface().toColor(),
        surfaceVariant = dyn.surfaceVariant().toColor(),
        onSurfaceVariant = dyn.onSurfaceVariant().toColor(),
        surfaceTint = dyn.surfaceTint().toColor(),
        inverseSurface = dyn.inverseSurface().toColor(),
        inverseOnSurface = dyn.inverseOnSurface().toColor(),
        error = dyn.error().toColor(),
        onError = dyn.onError().toColor(),
        errorContainer = dyn.errorContainer().toColor(),
        onErrorContainer = dyn.onErrorContainer().toColor(),
        outline = dyn.outline().toColor(),
        outlineVariant = dyn.outlineVariant().toColor(),
        scrim = dyn.scrim().toColor(),
        surfaceBright = dyn.surfaceBright().toColor(),
        surfaceDim = dyn.surfaceDim().toColor(),
        surfaceContainer = dyn.surfaceContainer().toColor(),
        surfaceContainerHigh = dyn.surfaceContainerHigh().toColor(),
        surfaceContainerHighest = dyn.surfaceContainerHighest().toColor(),
        surfaceContainerLow = dyn.surfaceContainerLow().toColor(),
        surfaceContainerLowest = dyn.surfaceContainerLowest().toColor(),
        primaryFixed = dyn.primaryFixed().toColor(),
        primaryFixedDim = dyn.primaryFixedDim().toColor(),
        onPrimaryFixed = dyn.onPrimaryFixed().toColor(),
        onPrimaryFixedVariant = dyn.onPrimaryFixedVariant().toColor(),
        secondaryFixed = dyn.secondaryFixed().toColor(),
        secondaryFixedDim = dyn.secondaryFixedDim().toColor(),
        onSecondaryFixed = dyn.onSecondaryFixed().toColor(),
        onSecondaryFixedVariant = dyn.onSecondaryFixedVariant().toColor(),
        tertiaryFixed = dyn.tertiaryFixed().toColor(),
        tertiaryFixedDim = dyn.tertiaryFixedDim().toColor(),
        onTertiaryFixed = dyn.onTertiaryFixed().toColor(),
        onTertiaryFixedVariant = dyn.onTertiaryFixedVariant().toColor(),
    )
}

val MagiskAccentColor = Color(0xFF00AF9C)

object ThemeState {
    var colorMode by mutableIntStateOf(Config.colorMode)
}

// Same values as androidx.activity's internal DefaultLightScrim / DefaultDarkScrim
private val LightScrim = android.graphics.Color.argb(0xe6, 0xFF, 0xFF, 0xFF)
private val DarkScrim = android.graphics.Color.argb(0x80, 0x1b, 0x1b, 0x1b)

/**
 * Enable edge-to-edge with system bar appearance derived from the app's color mode
 * instead of the system's night mode.
 */
fun ComponentActivity.applySystemBarStyle() {
    val isDark = { res: Resources ->
        val nightMode = res.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK
        ColorMode.fromValue(ThemeState.colorMode).isDark(nightMode == Configuration.UI_MODE_NIGHT_YES)
    }
    enableEdgeToEdge(
        statusBarStyle = SystemBarStyle.auto(
            android.graphics.Color.TRANSPARENT, android.graphics.Color.TRANSPARENT, isDark
        ),
        navigationBarStyle = SystemBarStyle.auto(LightScrim, DarkScrim, isDark),
    )
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
        window.isNavigationBarContrastEnforced = false
    }
}

val MagiskShapes = Shapes(
    extraSmall = RoundedCornerShape(6.dp),
    small = RoundedCornerShape(10.dp),
    medium = RoundedCornerShape(16.dp),
    large = RoundedCornerShape(24.dp),
    extraLarge = RoundedCornerShape(32.dp),
)

val MagiskTypography = Typography(
    headlineLarge = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 32.sp,
        lineHeight = 40.sp,
        letterSpacing = (-0.25).sp,
    ),
    headlineMedium = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 28.sp,
        lineHeight = 36.sp,
    ),
    headlineSmall = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 24.sp,
        lineHeight = 32.sp,
    ),
    titleLarge = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 20.sp,
        lineHeight = 26.sp,
    ),
    titleMedium = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.15.sp,
    ),
    titleSmall = TextStyle(
        fontWeight = FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
    ),
    bodyLarge = TextStyle(
        fontWeight = FontWeight.Normal,
        fontSize = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.5.sp,
    ),
    bodyMedium = TextStyle(
        fontWeight = FontWeight.Normal,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.25.sp,
    ),
    bodySmall = TextStyle(
        fontWeight = FontWeight.Normal,
        fontSize = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.4.sp,
    ),
    labelLarge = TextStyle(
        fontWeight = FontWeight.SemiBold,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
    ),
    labelMedium = TextStyle(
        fontWeight = FontWeight.Medium,
        fontSize = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
    ),
    labelSmall = TextStyle(
        fontWeight = FontWeight.Medium,
        fontSize = 11.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
    ),
)

@Composable
fun MagiskTheme(
    content: @Composable () -> Unit
) {
    val isDark = isSystemInDarkTheme()
    val mode = ColorMode.fromValue(ThemeState.colorMode)
    val context = LocalContext.current

    val isDarkTheme = mode.isDark(isDark)
    val useDynamicColor = mode.isMonet && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S

    val colorScheme = when {
        useDynamicColor && isDarkTheme -> dynamicDarkColorScheme(context)
        useDynamicColor && !isDarkTheme -> dynamicLightColorScheme(context)
        isDarkTheme -> dynamicColorScheme(MagiskAccentColor, isDark = true)
        else -> dynamicColorScheme(MagiskAccentColor, isDark = false)
    }

    val activity = LocalActivity.current as? ComponentActivity
    if (activity != null) {
        LaunchedEffect(activity, isDarkTheme) {
            activity.applySystemBarStyle()
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        shapes = MagiskShapes,
        typography = MagiskTypography,
        content = content
    )
}
