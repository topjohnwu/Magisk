package com.topjohnwu.magisk.dialog

import android.widget.TextView
import androidx.annotation.CallSuper
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.view.MagiskDialog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import top.yukonga.miuix.kmp.basic.Text
import java.io.IOException
import com.topjohnwu.magisk.core.R as CoreR

abstract class MarkDownDialog : DialogBuilder {

    abstract suspend fun getMarkdownText(): String

    @CallSuper
    override fun build(dialog: MagiskDialog) {
        dialog.setView {
            MarkdownContent(::getMarkdownText)
        }
    }
}

@Composable
private fun MarkdownContent(getMarkdownText: suspend () -> String) {
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
        error -> Text(stringResource(CoreR.string.download_file_error))
        mdText != null -> {
            val text = mdText!!
            AndroidView(
                factory = { context ->
                    TextView(context).apply {
                        ServiceLocator.markwon.setMarkdown(this, text)
                    }
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .heightIn(max = 300.dp)
            )
        }
    }
}
