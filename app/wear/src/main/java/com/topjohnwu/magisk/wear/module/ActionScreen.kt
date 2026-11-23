package com.topjohnwu.magisk.wear.module

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
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.material3.AlertDialog
import androidx.wear.compose.material3.AlertDialogDefaults
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.wear.nav.BlockDismiss
import com.topjohnwu.magisk.core.R as CoreR

/**
 * The output of a module's action.sh. While it runs, the screen can't be swiped away; back
 * (and from API 36 the swipe, which is predictive back) asks before leaving, like the phone
 * app, so a script that hangs can still be left.
 */
@Composable
fun ActionScreen(
    viewModel: ActionViewModel,
    actionName: String,
    onLeave: () -> Unit,
) {
    val actionState by viewModel.actionState.collectAsStateWithLifecycle()
    val running = actionState == ActionViewModel.State.RUNNING
    var showExit by rememberSaveable { mutableStateOf(false) }

    BlockDismiss(blocked = running, onBlockedBack = { showExit = true })

    ScreenScaffold { contentPadding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(contentPadding),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(
                text = actionName,
                style = MaterialTheme.typography.titleSmall,
                textAlign = TextAlign.Center,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
            val colors = MaterialTheme.colorScheme
            Text(
                text = stringResource(
                    when (actionState) {
                        ActionViewModel.State.RUNNING -> CoreR.string.wear_action_running
                        ActionViewModel.State.SUCCESS -> CoreR.string.done
                        ActionViewModel.State.FAILED -> CoreR.string.failure
                    }
                ),
                style = MaterialTheme.typography.labelSmall,
                color = when (actionState) {
                    ActionViewModel.State.RUNNING -> colors.onSurfaceVariant
                    ActionViewModel.State.SUCCESS -> colors.primary
                    ActionViewModel.State.FAILED -> colors.error
                },
            )
            TerminalText(
                emu = viewModel.emulator,
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth()
                    .padding(horizontal = 8.dp, vertical = 4.dp),
                onResized = viewModel::onTerminalResized,
            )
        }
    }

    AlertDialog(
        // Not shown any more once the action has finished meanwhile: back just leaves then
        visible = showExit && running,
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
