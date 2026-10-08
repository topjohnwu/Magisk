package com.topjohnwu.magisk.ui.component

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.material3.adaptive.currentWindowAdaptiveInfo
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.window.core.layout.WindowSizeClass

/**
 * Shows [first] and [second] side by side as two equal columns, centered and capped in width,
 * on expanded width windows. Otherwise, shows them one after the other in a single column.
 */
@Composable
fun AdaptiveColumns(
    modifier: Modifier = Modifier,
    spacing: Dp = 12.dp,
    first: @Composable ColumnScope.() -> Unit,
    second: @Composable ColumnScope.() -> Unit,
) {
    val twoColumns = currentWindowAdaptiveInfo().windowSizeClass
        .isWidthAtLeastBreakpoint(WindowSizeClass.WIDTH_DP_EXPANDED_LOWER_BOUND)
    val arrangement = Arrangement.spacedBy(spacing)
    if (twoColumns) {
        Row(
            modifier = modifier
                .fillMaxWidth()
                .wrapContentWidth(Alignment.CenterHorizontally)
                .widthIn(max = 1200.dp),
            horizontalArrangement = arrangement,
        ) {
            Column(Modifier.weight(1f), verticalArrangement = arrangement, content = first)
            Column(Modifier.weight(1f), verticalArrangement = arrangement, content = second)
        }
    } else {
        Column(modifier, verticalArrangement = arrangement) {
            first()
            second()
        }
    }
}
