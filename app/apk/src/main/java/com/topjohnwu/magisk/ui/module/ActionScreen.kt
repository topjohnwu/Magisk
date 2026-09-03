package com.topjohnwu.magisk.ui.module

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ui.component.rememberExternalStoragePermissionLauncher
import com.topjohnwu.magisk.ui.terminal.TerminalScreen
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ActionScreen(
    viewModel: ActionViewModel,
    actionName: String,
    onBack: () -> Unit,
    modifier: Modifier = Modifier
) {
    val actionState by viewModel.actionState.collectAsStateWithLifecycle()
    val finished = actionState != ActionViewModel.State.RUNNING
    val saveLog = rememberExternalStoragePermissionLauncher {
        viewModel.saveLog()
    }

    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(actionName) },
                navigationIcon = {
                    IconButton(
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = null,
                        )
                    }
                },
                actions = {
                    if (finished) {
                        IconButton(
                            onClick = saveLog
                        ) {
                            Icon(
                                painter = painterResource(R.drawable.ic_save),
                                contentDescription = stringResource(CoreR.string.menuSaveLog),
                            )
                        }
                    }
                },
                scrollBehavior = scrollBehavior
            )
        }
    ) { padding ->
        TerminalScreen(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding),
            onEmulatorCreated = { viewModel.onEmulatorCreated(it) },
        )
    }
}
