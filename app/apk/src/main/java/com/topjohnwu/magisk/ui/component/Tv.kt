package com.topjohnwu.magisk.ui.component

import android.content.ActivityNotFoundException
import android.content.Context
import android.widget.Toast
import androidx.activity.compose.ManagedActivityResultLauncher
import androidx.compose.foundation.layout.padding
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.R as CoreR

/**
 * Many Android TV images ship without a document picker, so launching one throws
 * [ActivityNotFoundException]. Report it instead of crashing the app.
 */
fun <I> ManagedActivityResultLauncher<I, *>.launchSafely(context: Context, input: I): Boolean {
    return try {
        launch(input)
        true
    } catch (e: ActivityNotFoundException) {
        context.toast(CoreR.string.app_not_found, Toast.LENGTH_LONG)
        false
    }
}

/**
 * TVs may overscan, so keep content inside the title-safe area (about 5%
 * of a 960x540dp screen). No-op on phones and tablets.
 */
fun Modifier.tvSafeArea(): Modifier =
    if (Info.isTv) padding(horizontal = 48.dp, vertical = 27.dp) else this
