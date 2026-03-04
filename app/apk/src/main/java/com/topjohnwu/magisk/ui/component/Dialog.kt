package com.topjohnwu.magisk.ui.component

import android.widget.TextView
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.WindowInsetsSides
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.only
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.Layout
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.viewinterop.AndroidView
import com.topjohnwu.magisk.core.di.ServiceLocator
import kotlinx.coroutines.CancellableContinuation
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.consumeAsFlow
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import timber.log.Timber
import top.yukonga.miuix.kmp.basic.ButtonDefaults
import top.yukonga.miuix.kmp.basic.InfiniteProgressIndicator
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.extra.SuperDialog
import top.yukonga.miuix.kmp.theme.MiuixTheme
import java.io.IOException
import kotlin.coroutines.resume

sealed interface ConfirmResult {
    data object Confirmed : ConfirmResult
    data object Canceled : ConfirmResult
}

data class DialogVisuals(
    val title: String = "",
    val content: String? = null,
    val markdown: Boolean = false,
    val confirm: String? = null,
    val dismiss: String? = null,
)

interface LoadingDialogHandle {
    suspend fun <R> withLoading(block: suspend () -> R): R
}

interface ConfirmDialogHandle {
    fun showConfirm(
        title: String,
        content: String? = null,
        markdown: Boolean = false,
        confirm: String? = null,
        dismiss: String? = null
    )

    suspend fun awaitConfirm(
        title: String,
        content: String? = null,
        markdown: Boolean = false,
        confirm: String? = null,
        dismiss: String? = null
    ): ConfirmResult
}

private class LoadingDialogHandleImpl(
    private val visible: MutableState<Boolean>,
    private val coroutineScope: CoroutineScope
) : LoadingDialogHandle {
    override suspend fun <R> withLoading(block: suspend () -> R): R {
        return coroutineScope.async {
            try {
                visible.value = true
                block()
            } finally {
                visible.value = false
            }
        }.await()
    }
}

private class ConfirmDialogHandleImpl(
    private val visible: MutableState<Boolean>,
    private val coroutineScope: CoroutineScope,
    private val callback: ConfirmCallback,
    private val resultChannel: Channel<ConfirmResult>
) : ConfirmDialogHandle {

    var visuals by mutableStateOf(DialogVisuals())
        private set

    private var awaitContinuation: CancellableContinuation<ConfirmResult>? = null

    init {
        coroutineScope.launch {
            resultChannel
                .consumeAsFlow()
                .onEach { result ->
                    awaitContinuation?.let {
                        awaitContinuation = null
                        if (it.isActive) it.resume(result)
                    }
                }
                .onEach { visible.value = false }
                .collect { result ->
                    when (result) {
                        ConfirmResult.Confirmed -> callback.onConfirm?.invoke()
                        ConfirmResult.Canceled -> callback.onDismiss?.invoke()
                    }
                }
        }
    }

    override fun showConfirm(
        title: String,
        content: String?,
        markdown: Boolean,
        confirm: String?,
        dismiss: String?
    ) {
        coroutineScope.launch {
            visuals = DialogVisuals(title, content, markdown, confirm, dismiss)
            visible.value = true
        }
    }

    override suspend fun awaitConfirm(
        title: String,
        content: String?,
        markdown: Boolean,
        confirm: String?,
        dismiss: String?
    ): ConfirmResult {
        coroutineScope.launch {
            visuals = DialogVisuals(title, content, markdown, confirm, dismiss)
            visible.value = true
        }
        return suspendCancellableCoroutine { cont ->
            awaitContinuation = cont.apply {
                invokeOnCancellation { visible.value = false }
            }
        }
    }
}

interface ConfirmCallback {
    val onConfirm: (() -> Unit)?
    val onDismiss: (() -> Unit)?
}

@Composable
fun rememberConfirmCallback(
    onConfirm: (() -> Unit)? = null,
    onDismiss: (() -> Unit)? = null
): ConfirmCallback {
    val currentOnConfirm by rememberUpdatedState(onConfirm)
    val currentOnDismiss by rememberUpdatedState(onDismiss)
    return remember {
        object : ConfirmCallback {
            override val onConfirm get() = currentOnConfirm
            override val onDismiss get() = currentOnDismiss
        }
    }
}

@Composable
fun rememberLoadingDialog(): LoadingDialogHandle {
    val visible = remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()
    LoadingDialog(visible)
    return remember { LoadingDialogHandleImpl(visible, scope) }
}

@Composable
fun rememberConfirmDialog(
    onConfirm: (() -> Unit)? = null,
    onDismiss: (() -> Unit)? = null
): ConfirmDialogHandle {
    return rememberConfirmDialog(rememberConfirmCallback(onConfirm, onDismiss))
}

@Composable
fun rememberConfirmDialog(callback: ConfirmCallback): ConfirmDialogHandle {
    val visible = rememberSaveable { mutableStateOf(false) }
    val scope = rememberCoroutineScope()
    val resultChannel = remember { Channel<ConfirmResult>() }

    val handle = remember {
        ConfirmDialogHandleImpl(visible, scope, callback, resultChannel)
    }

    if (visible.value) {
        ConfirmDialogContent(
            visuals = handle.visuals,
            confirm = { scope.launch { resultChannel.send(ConfirmResult.Confirmed) } },
            dismiss = { scope.launch { resultChannel.send(ConfirmResult.Canceled) } },
            showDialog = visible
        )
    }

    return handle
}

@Composable
private fun LoadingDialog(showDialog: MutableState<Boolean>) {
    SuperDialog(
        show = showDialog,
        onDismissRequest = {},
        content = {
            Box(
                modifier = Modifier.fillMaxWidth(),
                contentAlignment = Alignment.CenterStart
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.Start,
                ) {
                    InfiniteProgressIndicator(
                        color = MiuixTheme.colorScheme.onBackground
                    )
                    Text(
                        modifier = Modifier.padding(start = 12.dp),
                        text = stringResource(com.topjohnwu.magisk.core.R.string.loading),
                    )
                }
            }
        }
    )
}

@Composable
private fun ConfirmDialogContent(
    visuals: DialogVisuals,
    confirm: () -> Unit,
    dismiss: () -> Unit,
    showDialog: MutableState<Boolean>
) {
    SuperDialog(
        modifier = Modifier.windowInsetsPadding(WindowInsets.systemBars.only(WindowInsetsSides.Top)),
        show = showDialog,
        title = visuals.title,
        onDismissRequest = {
            dismiss()
            showDialog.value = false
        },
        content = {
            Layout(
                content = {
                    visuals.content?.let { content ->
                        if (visuals.markdown) {
                            MarkdownText(content)
                        } else {
                            Text(
                                text = content,
                                color = MiuixTheme.colorScheme.onSurface,
                            )
                        }
                    }
                    Row(
                        horizontalArrangement = Arrangement.SpaceBetween,
                        modifier = Modifier.padding(top = 12.dp)
                    ) {
                        TextButton(
                            text = visuals.dismiss
                                ?: stringResource(android.R.string.cancel),
                            onClick = {
                                dismiss()
                                showDialog.value = false
                            },
                            modifier = Modifier.weight(1f)
                        )
                        Spacer(Modifier.width(20.dp))
                        TextButton(
                            text = visuals.confirm
                                ?: stringResource(android.R.string.ok),
                            onClick = {
                                confirm()
                                showDialog.value = false
                            },
                            modifier = Modifier.weight(1f),
                            colors = ButtonDefaults.textButtonColorsPrimary()
                        )
                    }
                }
            ) { measurables, constraints ->
                if (measurables.size != 2) {
                    val button = measurables[0].measure(constraints)
                    layout(constraints.maxWidth, button.height) {
                        button.place(0, 0)
                    }
                } else {
                    val button = measurables[1].measure(constraints)
                    val content = measurables[0].measure(
                        constraints.copy(maxHeight = constraints.maxHeight - button.height)
                    )
                    layout(constraints.maxWidth, content.height + button.height) {
                        content.place(0, 0)
                        button.place(0, content.height)
                    }
                }
            }
        }
    )
}

@Composable
fun MarkdownText(text: String) {
    val contentColor = MiuixTheme.colorScheme.onBackground.toArgb()
    AndroidView(
        factory = { context ->
            TextView(context).apply {
                setTextColor(contentColor)
                ServiceLocator.markwon.setMarkdown(this, text)
            }
        },
        update = { textView ->
            textView.setTextColor(contentColor)
            ServiceLocator.markwon.setMarkdown(textView, text)
        },
        modifier = Modifier
            .fillMaxWidth()
            .heightIn(max = 300.dp)
    )
}

@Composable
fun MarkdownTextAsync(getMarkdownText: suspend () -> String) {
    var mdText by remember { mutableStateOf<String?>(null) }
    var error by remember { mutableStateOf(false) }

    LaunchedEffect(Unit) {
        try {
            mdText = withContext(Dispatchers.IO) { getMarkdownText() }
        } catch (e: IOException) {
            Timber.e(e)
            error = true
        }
    }

    when {
        error -> Text(stringResource(com.topjohnwu.magisk.core.R.string.download_file_error))
        mdText != null -> MarkdownText(mdText!!)
        else -> Box(
            modifier = Modifier.fillMaxWidth(),
            contentAlignment = Alignment.Center
        ) {
            InfiniteProgressIndicator(color = MiuixTheme.colorScheme.onBackground)
        }
    }
}
