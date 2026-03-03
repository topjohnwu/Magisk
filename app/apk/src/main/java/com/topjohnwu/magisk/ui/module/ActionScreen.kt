package com.topjohnwu.magisk.ui.module

import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Box
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
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import top.yukonga.miuix.kmp.basic.FloatingActionButton
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun ActionScreen(viewModel: ActionViewModel, onClose: () -> Unit) {
    val actionState by viewModel.actionState.collectAsState()
    val items = viewModel.consoleItems
    val listState = rememberLazyListState()

    LaunchedEffect(items.size) {
        if (items.isNotEmpty()) {
            listState.animateScrollToItem(items.size - 1)
        }
    }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            state = listState,
            modifier = Modifier
                .fillMaxSize()
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

        if (actionState != ActionViewModel.State.RUNNING) {
            TextButton(
                text = stringResource(CoreR.string.menuSaveLog),
                onClick = { viewModel.saveLog() },
                modifier = Modifier
                    .align(Alignment.BottomStart)
                    .padding(16.dp)
            )

            FloatingActionButton(
                onClick = onClose,
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(16.dp)
            ) {
                Text(
                    text = stringResource(CoreR.string.close),
                    modifier = Modifier.padding(horizontal = 16.dp)
                )
            }
        }
    }
}
