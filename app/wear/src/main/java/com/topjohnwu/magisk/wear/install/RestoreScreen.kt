package com.topjohnwu.magisk.wear.install

import android.widget.Toast
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.keepScreenOn
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.navigation3.runtime.NavKey
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.wear.nav.BlockDismiss
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.theme.destructiveButtonColors
import com.topjohnwu.magisk.core.R as CoreR

/**
 * Warns before restoring the stock images, which flashes back the boot image backup that
 * Magisk saved when it patched the installed one, and shows that backup's SHA1. Restoring has
 * to be confirmed by holding the button, so a stray tap can't start it.
 */
@Composable
fun RestoreScreen(viewModel: HomeViewModel, navigator: WearNavigator, key: NavKey) {
    val context = LocalContext.current
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    // null while loading
    var backup by remember { mutableStateOf<BackupInfo?>(null) }
    var started by rememberSaveable { mutableStateOf(false) }
    val restoring = uiState.showLoading

    LaunchedEffect(Unit) {
        backup = BackupInfo.read()
    }

    // Leave once the restore has run: the view model shows its result as a toast
    LaunchedEffect(restoring) {
        if (restoring) {
            started = true
        } else if (started && navigator.current() == key) {
            navigator.pop()
        }
    }

    BlockDismiss(blocked = restoring, onBlockedBack = {
        context.toast(CoreR.string.wear_flash_wait, Toast.LENGTH_SHORT)
    })

    val state = rememberTransformingLazyColumnState()

    if (restoring) {
        // Keep the screen on while the images are flashed back, like FlashScreen
        LoadingScreen(modifier = Modifier.keepScreenOn())
        return
    }

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.restore_img)) }
            }
            item {
                Text(
                    text = stringResource(CoreR.string.wear_restore_msg),
                    style = MaterialTheme.typography.bodySmall,
                    textAlign = TextAlign.Center,
                )
            }
            item {
                BackupCard(backup)
            }
            item {
                Button(
                    onClick = { context.toast(CoreR.string.wear_restore_hold, Toast.LENGTH_SHORT) },
                    onLongClick = { viewModel.restoreImages() },
                    onLongClickLabel = stringResource(CoreR.string.restore_img),
                    enabled = backup?.exists == true,
                    colors = destructiveButtonColors(),
                    label = { Text(stringResource(CoreR.string.restore_img)) },
                    secondaryLabel = { Text(stringResource(CoreR.string.wear_restore_hold)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
private fun BackupCard(backup: BackupInfo?) {
    val colors = MaterialTheme.colorScheme
    Column(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        when {
            backup == null -> Text(
                text = stringResource(CoreR.string.loading),
                style = MaterialTheme.typography.bodySmall,
                color = colors.onSurfaceVariant,
            )
            backup.exists && backup.sha1 != null -> {
                Text(
                    text = stringResource(CoreR.string.wear_restore_sha1),
                    style = MaterialTheme.typography.labelSmall,
                    color = colors.onSurfaceVariant,
                )
                Text(
                    text = backup.sha1,
                    style = MaterialTheme.typography.bodySmall.copy(
                        fontFamily = FontFamily.Monospace
                    ),
                    textAlign = TextAlign.Center,
                )
            }
            else -> Text(
                text = stringResource(CoreR.string.wear_restore_no_backup),
                style = MaterialTheme.typography.bodySmall,
                color = colors.error,
                textAlign = TextAlign.Center,
            )
        }
    }
}
