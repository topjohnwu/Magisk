package com.topjohnwu.magisk.wear

import android.content.Context
import android.content.res.Configuration
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.ui.Modifier
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.TimeText
import androidx.wear.compose.material3.timeTextCurvedText

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
 * its content to this class instead of showing the phone UI. The activity keeps doing the
 * common splash and initialization sequence.
 */
class WearUi(private val activity: ComponentActivity) {

    fun onCreateUi(savedInstanceState: Bundle?) {
        activity.setContent {
            MaterialTheme {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(MaterialTheme.colorScheme.background)
                ) {
                    TimeText { time -> timeTextCurvedText(time) }
                }
            }
        }
    }
}
