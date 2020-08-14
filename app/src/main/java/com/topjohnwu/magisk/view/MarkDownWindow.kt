package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.ktx.coroutineScope
import io.noties.markwon.Markwon
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import kotlin.coroutines.coroutineContext

object MarkDownWindow : KoinComponent {

    private val repo: StringRepository by inject()
    private val markwon: Markwon by inject()

    suspend fun show(activity: Context, title: String?, url: String) {
        show(activity, title) {
            repo.getString(url)
        }
    }

    suspend fun show(activity: Context, title: String?, input: suspend () -> String) {
        val view = LayoutInflater.from(activity).inflate(R.layout.markdown_window_md2, null)

        MagiskDialog(activity)
            .applyTitle(title ?: "")
            .applyView(view)
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.cancel
            }
            .reveal()

        val tv = view.findViewById<TextView>(R.id.md_txt)
        tv.coroutineScope = CoroutineScope(coroutineContext)
        withContext(Dispatchers.IO) {
            try {
                markwon.setMarkdown(tv, input())
            } catch (e: Exception) {
                if (e is CancellationException)
                    throw e
                Timber.e(e)
                tv.post { tv.setText(R.string.download_file_error) }
            }
        }
    }
}
