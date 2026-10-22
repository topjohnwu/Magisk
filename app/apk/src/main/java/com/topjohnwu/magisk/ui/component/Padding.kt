package com.topjohnwu.magisk.ui.component

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.calculateEndPadding
import androidx.compose.foundation.layout.calculateStartPadding
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalLayoutDirection

/**
 * The start/end part of these paddings, e.g. the side system bars in landscape.
 * Useful when the top/bottom parts are applied separately as scrollable content padding.
 */
@Composable
fun PaddingValues.onlyHorizontal(): PaddingValues {
    val layoutDirection = LocalLayoutDirection.current
    return PaddingValues(
        start = calculateStartPadding(layoutDirection),
        end = calculateEndPadding(layoutDirection),
    )
}
