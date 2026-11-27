package com.topjohnwu.magisk.wear.install

import android.widget.Toast
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.keepScreenOn
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.CircularProgressIndicator
import androidx.wear.compose.material3.CompactButton
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.wear.module.TerminalText
import com.topjohnwu.magisk.wear.nav.BlockDismiss
import com.topjohnwu.magisk.core.R as CoreR

/**
 * The output of an installation, with an indeterminate progress ring on the edge of the screen
 * while it runs.
 *
 * While it runs, the screen can't be swiped away. Flashing or patching a boot image can't be
 * left at all: stopping halfway is never safer than waiting. Only a module zip, which runs a
 * script from the module that may hang, can be left after a confirmation, like the phone app.
 */
@Composable
fun FlashScreen(
    viewModel: FlashViewModel,
    action: String,
    onLeave: () -> Unit,
) {
    val context = LocalContext.current
    val flashState by viewModel.flashState.collectAsStateWithLifecycle()
    val showReboot by viewModel.showReboot.collectAsStateWithLifecycle()
    val flashing = flashState == FlashViewModel.State.FLASHING
    val leavable = action == Const.Value.FLASH_ZIP
    var showExit by rememberSaveable { mutableStateOf(false) }

    BlockDismiss(blocked = flashing, onBlockedBack = {
        if (leavable) {
            showExit = true
        } else {
            context.toast(CoreR.string.wear_flash_wait, Toast.LENGTH_SHORT)
        }
    })

    ScreenScaffold { contentPadding ->
        // Watches turn the screen off quickly. Keep it on while this runs, so the app stays in
        // the foreground and is not killed for memory halfway through.
        Box(
            modifier = Modifier
                .fillMaxSize()
                .then(if (flashing) Modifier.keepScreenOn() else Modifier)
        ) {
            if (flashing) {
                CircularProgressIndicator(modifier = Modifier.fillMaxSize())
            }
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(contentPadding),
                horizontalAlignment = Alignment.CenterHorizontally,
            ) {
                Text(
                    text = stringResource(CoreR.string.flash_screen_title),
                    style = MaterialTheme.typography.titleSmall,
                    textAlign = TextAlign.Center,
                )
                val colors = MaterialTheme.colorScheme
                Text(
                    text = stringResource(
                        when (flashState) {
                            FlashViewModel.State.FLASHING -> CoreR.string.flashing
                            FlashViewModel.State.SUCCESS -> CoreR.string.done
                            FlashViewModel.State.FAILED -> CoreR.string.failure
                        }
                    ),
                    style = MaterialTheme.typography.labelSmall,
                    color = when (flashState) {
                        FlashViewModel.State.FLASHING -> colors.onSurfaceVariant
                        FlashViewModel.State.SUCCESS -> colors.primary
                        FlashViewModel.State.FAILED -> colors.error
                    },
                )
                TerminalText(
                    emu = viewModel.emulator,
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    // A module install waits for this to start with the real terminal size
                    onResized = viewModel::onTerminalResized,
                )
                if (flashing) {
                    Text(
                        text = stringResource(CoreR.string.wear_flash_keep_charging),
                        style = MaterialTheme.typography.labelSmall,
                        color = colors.onSurfaceVariant,
                        textAlign = TextAlign.Center,
                    )
                } else if (flashState == FlashViewModel.State.SUCCESS && showReboot) {
                    CompactButton(
                        onClick = { viewModel.restartPressed() },
                        label = { Text(stringResource(CoreR.string.reboot)) },
                    )
                }
            }
        }
    }

    AlertDialog(
        // Not shown any more once the installation has finished meanwhile: back just leaves then
        visible = showExit && flashing,
        onDismissRequest = { showExit = false },
        title = { Text(stringResource(CoreR.string.running_exit_title)) },
        text = { Text(stringResource(CoreR.string.running_exit_msg)) },
        confirmButton = {
            AlertDialogDefaults.ConfirmButton(onClick = {
                showExit = false
                onLeave()
            })
        },
        dismissButton = {
            AlertDialogDefaults.DismissButton(onClick = { showExit = false })
        },
    )
}
