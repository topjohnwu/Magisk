package com.topjohnwu.magisk.ui.flash

import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.ui.terminal.TerminalComposeView
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.SmallTopAppBar
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Back
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun FlashScreen(viewModel: FlashViewModel, action: String, onBack: () -> Unit) {
    val flashState by viewModel.flashState.collectAsState()
    val showReboot by viewModel.showReboot.collectAsState()
    val finished = flashState != FlashViewModel.State.FLASHING
    val useTerminal = action == Const.Value.FLASH_ZIP

    val statusText = when (flashState) {
        FlashViewModel.State.FLASHING -> stringResource(CoreR.string.flashing)
        FlashViewModel.State.SUCCESS -> stringResource(CoreR.string.done)
        FlashViewModel.State.FAILED -> stringResource(CoreR.string.failure)
    }

    val scrollBehavior = MiuixScrollBehavior()
    Scaffold(
        topBar = {
            SmallTopAppBar(
                title = "${stringResource(CoreR.string.flash_screen_title)} - $statusText",
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = MiuixIcons.Back,
                            contentDescription = null,
                            tint = MiuixTheme.colorScheme.onBackground
                        )
                    }
                },
                actions = {
                    if (finished) {
                        IconButton(
                            modifier = Modifier.padding(end = 4.dp),
                            onClick = { viewModel.saveLog() }
                        ) {
                            Icon(
                                painter = painterResource(R.drawable.ic_save_md2),
                                contentDescription = stringResource(CoreR.string.menuSaveLog),
                                tint = MiuixTheme.colorScheme.onBackground
                            )
                        }
                    }
                    if (flashState == FlashViewModel.State.SUCCESS && showReboot) {
                        IconButton(
                            modifier = Modifier.padding(end = 16.dp),
                            onClick = { viewModel.restartPressed() }
                        ) {
                            Icon(
                                painter = painterResource(R.drawable.ic_restart),
                                contentDescription = stringResource(CoreR.string.reboot),
                                tint = MiuixTheme.colorScheme.onBackground
                            )
                        }
                    }
                },
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        if (useTerminal) {
            val session by viewModel.termSession.collectAsState()
            TerminalComposeView(
                session = session,
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                onViewCreated = { viewModel.setTerminalView(it) },
                onEmulatorReady = { viewModel.onEmulatorReady() },
            )
        } else {
            val items = viewModel.consoleItems
            val listState = rememberLazyListState()

            LaunchedEffect(items.size) {
                if (items.isNotEmpty()) {
                    listState.animateScrollToItem(items.size - 1)
                }
            }

            LazyColumn(
                state = listState,
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding)
                    .horizontalScroll(rememberScrollState())
                    .padding(horizontal = 8.dp, vertical = 4.dp)
            ) {
                itemsIndexed(items) { _, line ->
                    Text(
                        text = line,
                        fontFamily = FontFamily.Monospace,
                        fontSize = 12.sp,
                        lineHeight = 16.sp,
                        color = MiuixTheme.colorScheme.onSurface,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }
        }
    }
}
