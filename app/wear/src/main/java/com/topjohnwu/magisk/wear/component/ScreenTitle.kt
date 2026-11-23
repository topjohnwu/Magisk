package com.topjohnwu.magisk.wear.component

import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.Text

// Extra inset, as a fraction of the screen width, on top of the column's own horizontal padding.
// A round screen is narrowest at the top of a list, where this title sits.
private const val TopItemPaddingFraction = 0.14f

/**
 * The title of a detail screen, shown as the first item of its list. It wraps to more lines
 * instead of running into the edges of a round screen.
 */
@Composable
fun ScreenTitle(
    text: String,
    modifier: Modifier = Modifier,
    textDecoration: TextDecoration? = null,
) {
    Text(
        text = text,
        style = MaterialTheme.typography.titleMedium,
        textAlign = TextAlign.Center,
        textDecoration = textDecoration,
        modifier = modifier.padding(
            horizontal = (LocalConfiguration.current.screenWidthDp * TopItemPaddingFraction).dp
        ),
    )
}
