package com.topjohnwu.magisk.ui.component

import android.text.Selection
import android.text.Spannable
import android.text.method.LinkMovementMethod
import android.text.style.ClickableSpan
import android.view.MotionEvent
import android.view.ViewConfiguration
import android.widget.TextView
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import kotlin.math.abs
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.LocalContentColor
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ProvideTextStyle
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.topjohnwu.magisk.core.R
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
    LoadingDialog(visible = visible.value)
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
            confirm = {
                visible.value = false
                scope.launch { resultChannel.send(ConfirmResult.Confirmed) }
            },
            dismiss = {
                visible.value = false
                scope.launch { resultChannel.send(ConfirmResult.Canceled) }
            }
        )
    }

    return handle
}

@Composable
fun LoadingDialog(
    visible: Boolean,
    modifier: Modifier = Modifier
) {
    if (visible) {
        Dialog(
            onDismissRequest = {},
            properties = DialogProperties(dismissOnBackPress = false, dismissOnClickOutside = false)
        ) {
            Surface(
                shape = RoundedCornerShape(28.dp),
                color = MaterialTheme.colorScheme.surfaceContainerHigh,
                tonalElevation = 6.dp,
                modifier = modifier.widthIn(max = 320.dp)
            ) {
                Box(
                    modifier = Modifier.padding(24.dp).fillMaxWidth(),
                    contentAlignment = Alignment.CenterStart
                ) {
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.Start,
                    ) {
                        CircularProgressIndicator(
                            color = MaterialTheme.colorScheme.primary,
                        )
                        Text(
                            modifier = Modifier.padding(start = 16.dp),
                            text = stringResource(R.string.loading),
                            style = MaterialTheme.typography.bodyLarge,
                            color = MaterialTheme.colorScheme.onSurface,
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun MagiskDialog(
    onDismissRequest: () -> Unit,
    modifier: Modifier = Modifier,
    dismissOnClickOutside: Boolean = true,
    dismissOnBackPress: Boolean = true,
    title: @Composable (() -> Unit)? = null,
    confirmButton: @Composable (() -> Unit)? = null,
    dismissButton: @Composable (() -> Unit)? = null,
    neutralButton: @Composable (() -> Unit)? = null,
    scrollable: Boolean = false,
    content: @Composable () -> Unit,
) {
    Dialog(
        onDismissRequest = onDismissRequest,
        properties = DialogProperties(
            dismissOnClickOutside = dismissOnClickOutside,
            dismissOnBackPress = dismissOnBackPress,
            usePlatformDefaultWidth = false
        )
    ) {
        Surface(
            shape = RoundedCornerShape(28.dp),
            color = MaterialTheme.colorScheme.surfaceContainerHigh,
            tonalElevation = 6.dp,
            modifier = modifier
                .windowInsetsPadding(WindowInsets.systemBars)
                .padding(horizontal = 24.dp, vertical = 24.dp)
                .widthIn(min = 280.dp, max = 560.dp)
        ) {
            Column(modifier = Modifier.fillMaxWidth()) {
                title?.let {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, top = 24.dp, bottom = 16.dp)
                    ) {
                        CompositionLocalProvider(LocalContentColor provides MaterialTheme.colorScheme.onSurface) {
                            ProvideTextStyle(MaterialTheme.typography.titleLarge) {
                                it()
                            }
                        }
                    }
                }
                val hasButtons = confirmButton != null || dismissButton != null || neutralButton != null
                val contentModifier = Modifier
                    .weight(1f, fill = false)
                    .fillMaxWidth()
                    .then(
                        if (scrollable) {
                            val scrollState = rememberScrollState()
                            Modifier
                                .verticalScrollbar(
                                    scrollState,
                                    contentPadding = PaddingValues(top = 4.dp, bottom = 4.dp, end = 4.dp)
                                )
                                .verticalScroll(scrollState)
                        } else Modifier
                    )
                    .padding(
                        start = 24.dp,
                        end = 24.dp,
                        top = if (title == null) 24.dp else 0.dp,
                        bottom = if (hasButtons) 0.dp else 24.dp
                    )
                Box(modifier = contentModifier) {
                    CompositionLocalProvider(LocalContentColor provides MaterialTheme.colorScheme.onSurfaceVariant) {
                        ProvideTextStyle(MaterialTheme.typography.bodyMedium) {
                            content()
                        }
                    }
                }
                if (hasButtons) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, bottom = 24.dp, top = 16.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        if (neutralButton != null) {
                            neutralButton()
                            Spacer(Modifier.weight(1f))
                        } else {
                            Spacer(Modifier.weight(1f))
                        }
                        if (dismissButton != null) {
                            dismissButton()
                        }
                        if (dismissButton != null && confirmButton != null) {
                            Spacer(Modifier.width(8.dp))
                        }
                        if (confirmButton != null) {
                            confirmButton()
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun MagiskDialog(
    onDismissRequest: () -> Unit,
    title: String? = null,
    modifier: Modifier = Modifier,
    dismissOnClickOutside: Boolean = true,
    dismissOnBackPress: Boolean = true,
    confirmText: String? = null,
    onConfirm: (() -> Unit)? = null,
    confirmEnabled: Boolean = true,
    dismissText: String? = null,
    onDismiss: (() -> Unit)? = null,
    neutralText: String? = null,
    onNeutral: (() -> Unit)? = null,
    scrollable: Boolean = false,
    content: @Composable () -> Unit,
) {
    MagiskDialog(
        onDismissRequest = onDismissRequest,
        title = if (!title.isNullOrEmpty()) {
            { Text(text = title, style = MaterialTheme.typography.titleLarge) }
        } else null,
        modifier = modifier,
        dismissOnClickOutside = dismissOnClickOutside,
        dismissOnBackPress = dismissOnBackPress,
        confirmButton = if (confirmText != null || onConfirm != null) {
            {
                TextButton(
                    onClick = { onConfirm?.invoke() },
                    enabled = confirmEnabled,
                ) {
                    Text(
                        text = confirmText ?: stringResource(android.R.string.ok),
                        style = MaterialTheme.typography.labelLarge,
                    )
                }
            }
        } else null,
        dismissButton = if (dismissText != null || onDismiss != null) {
            {
                TextButton(onClick = { onDismiss?.invoke() }) {
                    Text(
                        text = dismissText ?: stringResource(android.R.string.cancel),
                        style = MaterialTheme.typography.labelLarge,
                    )
                }
            }
        } else null,
        neutralButton = if (neutralText != null && onNeutral != null) {
            {
                TextButton(onClick = onNeutral) {
                    Text(
                        text = neutralText,
                        style = MaterialTheme.typography.labelLarge,
                    )
                }
            }
        } else null,
        scrollable = scrollable,
        content = content
    )
}

@Composable
fun ConfirmDialogContent(
    visuals: DialogVisuals,
    confirm: () -> Unit,
    dismiss: () -> Unit,
    modifier: Modifier = Modifier
) {
    MagiskDialog(
        modifier = modifier,
        onDismissRequest = dismiss,
        title = visuals.title,
        confirmText = visuals.confirm,
        onConfirm = confirm,
        dismissText = visuals.dismiss,
        onDismiss = dismiss,
        scrollable = true,
    ) {
        visuals.content?.let { content ->
            if (visuals.markdown) {
                MarkdownText(content)
            } else {
                Text(text = content)
            }
        }
    }
}

private class LinkClickMovementMethod : LinkMovementMethod() {
    private var downX = 0f
    private var downY = 0f
    private var isDragging = false

    override fun onTouchEvent(widget: TextView, buffer: Spannable, event: MotionEvent): Boolean {
        val touchSlop = ViewConfiguration.get(widget.context).scaledTouchSlop

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                downX = event.x
                downY = event.y
                isDragging = false

                val link = findClickableSpan(widget, buffer, event.x, event.y)
                return if (link != null) {
                    Selection.setSelection(buffer, buffer.getSpanStart(link), buffer.getSpanEnd(link))
                    true
                } else {
                    Selection.removeSelection(buffer)
                    false
                }
            }
            MotionEvent.ACTION_MOVE -> {
                if (!isDragging) {
                    val dx = abs(event.x - downX)
                    val dy = abs(event.y - downY)
                    if (dx > touchSlop || dy > touchSlop) {
                        isDragging = true
                        Selection.removeSelection(buffer)
                    }
                }
                return false
            }
            MotionEvent.ACTION_UP -> {
                val hasSelection = Selection.getSelectionStart(buffer) != -1
                Selection.removeSelection(buffer)
                if (!isDragging && hasSelection) {
                    val link = findClickableSpan(widget, buffer, downX, downY)
                    if (link != null) {
                        link.onClick(widget)
                        return true
                    }
                }
                return false
            }
            MotionEvent.ACTION_CANCEL -> {
                Selection.removeSelection(buffer)
                isDragging = false
                return false
            }
        }
        return false
    }

    private fun findClickableSpan(widget: TextView, buffer: Spannable, x: Float, y: Float): ClickableSpan? {
        val xInt = (x - widget.totalPaddingLeft + widget.scrollX).toInt()
        val yInt = (y - widget.totalPaddingTop + widget.scrollY).toInt()

        val layout = widget.layout ?: return null
        if (yInt < 0 || yInt >= layout.height) return null

        val line = layout.getLineForVertical(yInt)
        if (xInt < layout.getLineLeft(line) || xInt > layout.getLineRight(line)) {
            return null
        }

        val offset = layout.getOffsetForHorizontal(line, xInt.toFloat())
        val links = buffer.getSpans(offset, offset, ClickableSpan::class.java)
        if (links.isEmpty()) return null

        val link = links[0]
        val start = buffer.getSpanStart(link)
        val end = buffer.getSpanEnd(link)

        val lineStart = layout.getLineStart(line)
        val lineEnd = layout.getLineEnd(line)
        val linkLineStart = maxOf(start, lineStart)
        val linkLineEnd = minOf(end, lineEnd)

        val left = layout.getPrimaryHorizontal(linkLineStart)
        val right = layout.getPrimaryHorizontal(linkLineEnd)
        val minX = minOf(left, right)
        val maxX = maxOf(left, right)

        return if (xInt in minX.toInt()..maxX.toInt()) link else null
    }
}

@Composable
fun MarkdownText(
    text: String,
    modifier: Modifier = Modifier,
    color: Color = MaterialTheme.colorScheme.onSurface,
    linkColor: Color = MaterialTheme.colorScheme.primary,
) {
    val contentColorArgb = color.toArgb()
    val linkColorArgb = linkColor.toArgb()
    val movementMethod = remember { LinkClickMovementMethod() }
    AndroidView(
        factory = { context ->
            TextView(context).apply {
                setTextColor(contentColorArgb)
                setLinkTextColor(linkColorArgb)
                ServiceLocator.markwon.setMarkdown(this, text)
                this.movementMethod = movementMethod
            }
        },
        update = { textView ->
            textView.setTextColor(contentColorArgb)
            textView.setLinkTextColor(linkColorArgb)
            ServiceLocator.markwon.setMarkdown(textView, text)
            textView.movementMethod = movementMethod
        },
        modifier = modifier.fillMaxWidth()
    )
}

@Composable
fun MarkdownTextAsync(
    modifier: Modifier = Modifier,
    getMarkdownText: suspend () -> String
) {
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
        error -> Text(
            text = stringResource(R.string.download_file_error),
            modifier = modifier
        )
        mdText != null -> MarkdownText(text = mdText!!, modifier = modifier)
        else -> Box(
            modifier = modifier.fillMaxWidth(),
            contentAlignment = Alignment.Center
        ) {
            CircularProgressIndicator()
        }
    }
}
