package com.topjohnwu.magisk.wear.theme

import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.wear.compose.material3.ButtonColors
import androidx.wear.compose.material3.ButtonDefaults
import androidx.wear.compose.material3.ColorScheme
import androidx.wear.compose.material3.MaterialTheme

private val WearColorScheme = ColorScheme(
    // One accent: switches and tonal buttons use the container colors
    primary = Color(0xFF4FD8C4),
    primaryDim = Color(0xFF3CB4A3),
    primaryContainer = Color(0xFF00504A),
    onPrimary = Color(0xFF00382F),
    onPrimaryContainer = Color(0xFF9FF2E3),
    // A black background keeps OLED pixels off and blends with the bezel
    background = Color.Black,
)

@Composable
fun WearTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = WearColorScheme, content = content)
}

/** Colors of a button that removes or restores something and can't be undone. */
@Composable
fun destructiveButtonColors(): ButtonColors {
    val colors = MaterialTheme.colorScheme
    return ButtonDefaults.buttonColors(
        containerColor = colors.error,
        contentColor = colors.onError,
        iconColor = colors.onError,
    )
}
