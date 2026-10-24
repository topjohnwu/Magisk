package com.topjohnwu.magisk.ui.component

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.pm.PackageManager
import android.os.Build
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.ui.platform.LocalContext
import androidx.core.content.ContextCompat
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.toast

@Composable
fun rememberExternalStoragePermissionLauncher(onGranted: () -> Unit): () -> Unit {
    val context = LocalContext.current
    val currentOnGranted by rememberUpdatedState(onGranted)

    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) {
            currentOnGranted()
        } else {
            context.toast(R.string.external_rw_permission_denied, Toast.LENGTH_SHORT)
        }
    }

    return remember(launcher, context) {
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R ||
                ContextCompat.checkSelfPermission(context, WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
            ) {
                currentOnGranted()
            } else {
                launcher.launch(WRITE_EXTERNAL_STORAGE)
            }
        }
    }
}

/**
 * Returns a function that runs [onGranted] once the app is allowed to post notifications,
 * asking for the permission first on Android 13+, and tells the user if it is denied.
 */
@Composable
fun rememberNotificationPermissionLauncher(onGranted: () -> Unit): () -> Unit {
    val context = LocalContext.current
    val currentOnGranted by rememberUpdatedState(onGranted)

    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) {
            currentOnGranted()
        } else {
            context.toast(R.string.post_notifications_denied, Toast.LENGTH_SHORT)
        }
    }

    return remember(launcher, context) {
        {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU ||
                ContextCompat.checkSelfPermission(context, POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED
            ) {
                currentOnGranted()
            } else {
                launcher.launch(POST_NOTIFICATIONS)
            }
        }
    }
}
