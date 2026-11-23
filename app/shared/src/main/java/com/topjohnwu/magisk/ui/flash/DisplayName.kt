package com.topjohnwu.magisk.ui.flash

import android.content.Context
import android.net.Uri
import android.os.CancellationSignal
import android.provider.OpenableColumns
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.withTimeoutOrNull

private const val MAX_DISPLAY_NAME_LENGTH = 100
private const val DISPLAY_NAME_QUERY_TIMEOUT_MS = 3000L

/**
 * Names from external apps are untrusted: drop control and formatting characters
 * (e.g. bidi overrides that can reorder the text) and cap the length.
 */
fun String.sanitizeDisplayName(): String {
    val sb = StringBuilder()
    var count = 0
    var i = 0
    // Work on code points, so supplementary characters (e.g. invisible tag characters)
    // are recognized and never split
    while (i < length && count < MAX_DISPLAY_NAME_LENGTH) {
        val cp = codePointAt(i)
        i += Character.charCount(cp)
        val type = Character.getType(cp)
        if (Character.isISOControl(cp) || type == Character.FORMAT.toInt() ||
            type == Character.SURROGATE.toInt()) continue
        sb.appendCodePoint(cp)
        count++
    }
    return sb.toString().trim()
}

/**
 * Query the display name from the provider of an external URI. The provider is controlled
 * by another app, so never block on it indefinitely: give up after a timeout and cancel it.
 */
suspend fun queryDisplayName(context: Context, uri: Uri): String? {
    val signal = CancellationSignal()
    // Not a child of the caller, so giving up doesn't have to wait for the provider
    val query = CoroutineScope(Dispatchers.IO).async {
        runCatching {
            context.contentResolver.query(
                uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null, signal
            )?.use { cursor ->
                val idx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                if (cursor.moveToFirst() && idx >= 0) cursor.getString(idx) else null
            }
        }.getOrNull()
    }
    return try {
        withTimeoutOrNull(DISPLAY_NAME_QUERY_TIMEOUT_MS) { query.await() }
    } finally {
        if (!query.isCompleted) {
            signal.cancel()
            query.cancel()
        }
    }
}
