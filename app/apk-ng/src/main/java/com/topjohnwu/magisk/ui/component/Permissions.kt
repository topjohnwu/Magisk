package com.topjohnwu.magisk.ui.component

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.widget.Toast
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.ui.platform.LocalContext
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.base.IActivityExtension
import com.topjohnwu.magisk.core.ktx.toast

@Composable
fun rememberExternalStoragePermissionLauncher(onGranted: () -> Unit): () -> Unit {
    val context = LocalContext.current
    val currentOnGranted by rememberUpdatedState(onGranted)

    return remember(context) {
        {
            (context as IActivityExtension).withPermission(WRITE_EXTERNAL_STORAGE) { granted ->
                if (granted) {
                    currentOnGranted()
                } else {
                    context.toast(R.string.external_rw_permission_denied, Toast.LENGTH_SHORT)
                }
            }
        }
    }
}
