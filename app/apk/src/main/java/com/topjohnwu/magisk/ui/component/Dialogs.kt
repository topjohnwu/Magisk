package com.topjohnwu.magisk.ui.component

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.State
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CancellableContinuation
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import kotlin.coroutines.resume
import com.topjohnwu.magisk.core.R as CoreR

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
) : LoadingDialogHandle {
    override suspend fun <R> withLoading(block: suspend () -> R): R {
        return try {
            withContext(Dispatchers.Main.immediate) { visible.value = true }
            block()
        } finally {
            withContext(Dispatchers.Main.immediate) { visible.value = false }
        }
    }
}

private class ConfirmDialogHandleImpl(
    private val visuals: MutableState<DialogVisuals?>,
) : ConfirmDialogHandle {

    var onResult: (ConfirmResult) -> Unit = {}
    private var awaitContinuation: CancellableContinuation<ConfirmResult>? = null

    override fun showConfirm(
        title: String,
        content: String?,
        markdown: Boolean,
        confirm: String?,
        dismiss: String?
    ) {
        visuals.value = DialogVisuals(title, content, markdown, confirm, dismiss)
    }

    override suspend fun awaitConfirm(
        title: String,
        content: String?,
        markdown: Boolean,
        confirm: String?,
        dismiss: String?
    ): ConfirmResult {
        visuals.value = DialogVisuals(title, content, markdown, confirm, dismiss)
        return suspendCancellableCoroutine { cont ->
            awaitContinuation?.cancel()
            awaitContinuation = cont
            cont.invokeOnCancellation {
                if (awaitContinuation === cont) {
                    awaitContinuation = null
                    visuals.value = null
                }
            }
        }
    }

    fun confirm() = complete(ConfirmResult.Confirmed)
    fun dismiss() = complete(ConfirmResult.Canceled)

    private fun complete(result: ConfirmResult) {
        visuals.value = null
        awaitContinuation?.takeIf { it.isActive }?.resume(result)
        awaitContinuation = null
        onResult(result)
    }
}

@Composable
fun rememberLoadingDialog(): LoadingDialogHandle {
    val visible = remember { mutableStateOf(false) }
    if (visible.value) {
        AlertDialog(
            onDismissRequest = {},
            title = null,
            text = {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    CircularProgressIndicator()
                    Text(text = stringResource(CoreR.string.loading))
                }
            },
            confirmButton = {}
        )
    }
    return remember { LoadingDialogHandleImpl(visible) }
}

@Composable
fun rememberConfirmDialog(
    onConfirm: (() -> Unit)? = null,
    onDismiss: (() -> Unit)? = null
): ConfirmDialogHandle {
    val currentOnConfirm by rememberUpdatedState(onConfirm)
    val currentOnDismiss by rememberUpdatedState(onDismiss)
    val visuals = remember { mutableStateOf<DialogVisuals?>(null) }
    val handle = remember { ConfirmDialogHandleImpl(visuals) }

    handle.onResult = { result ->
        when (result) {
            ConfirmResult.Confirmed -> currentOnConfirm?.invoke()
            ConfirmResult.Canceled -> currentOnDismiss?.invoke()
        }
    }

    ConfirmDialog(visuals = visuals, handle = handle)
    return handle
}

@Composable
private fun ConfirmDialog(
    visuals: State<DialogVisuals?>,
    handle: ConfirmDialogHandleImpl
) {
    val current = visuals.value ?: return
    AlertDialog(
        onDismissRequest = { handle.dismiss() },
        title = { Text(text = current.title) },
        text = {
            if (!current.content.isNullOrBlank()) {
                Text(text = current.content)
            }
        },
        dismissButton = {
            TextButton(onClick = { handle.dismiss() }) {
                Text(text = current.dismiss ?: stringResource(android.R.string.cancel))
            }
        },
        confirmButton = {
            TextButton(onClick = { handle.confirm() }) {
                Text(text = current.confirm ?: stringResource(android.R.string.ok))
            }
        }
    )
}
