package com.topjohnwu.magisk.view

import android.content.Context
import android.text.Spanned
import android.view.LayoutInflater
import android.widget.TextView
import androidx.core.text.PrecomputedTextCompat
import androidx.core.widget.TextViewCompat
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.StringRepository
import io.noties.markwon.Markwon
import kotlinx.coroutines.*
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import kotlin.coroutines.coroutineContext

class PrecomputedTextSetter : Markwon.TextSetter {

    override fun setText(tv: TextView, text: Spanned, bufferType: TextView.BufferType, onComplete: Runnable) {
        val scope = tv.tag as? CoroutineScope ?: GlobalScope
        scope.launch(Dispatchers.Default) {
            val pre = PrecomputedTextCompat.create(text, TextViewCompat.getTextMetricsParams(tv))
            tv.post {
                TextViewCompat.setPrecomputedText(tv, pre)
                onComplete.run()
            }
        }
    }
}

object MarkDownWindow : KoinComponent {

    private val repo: StringRepository by inject()
    private val markwon: Markwon by inject()

    suspend fun show(activity: Context, title: String?, url: String) {
        show(activity, title) {
            repo.getString(url)
        }
    }

    suspend fun show(activity: Context, title: String?, input: suspend () -> String) {
        val mdRes = R.layout.markdown_window_md2
        val mv = LayoutInflater.from(activity).inflate(mdRes, null)
        val tv = mv.findViewById<TextView>(R.id.md_txt)
        tv.tag = CoroutineScope(coroutineContext)

        try {
            markwon.setMarkdown(tv, input())
        } catch (e: Exception) {
            if (e is CancellationException)
                throw e
            Timber.e(e)
            tv.setText(R.string.download_file_error)
        }

        MagiskDialog(activity)
            .applyTitle(title ?: "")
            .applyView(mv)
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.cancel
            }
            .reveal()
    }
}
