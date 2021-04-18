package com.topjohnwu.magisk.events.dialog

import android.view.LayoutInflater
import android.widget.TextView
import androidx.annotation.CallSuper
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.view.MagiskDialog
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import kotlin.coroutines.cancellation.CancellationException

abstract class MarkDownDialog : DialogEvent() {

    abstract suspend fun getMarkdownText(): String

    @CallSuper
    override fun build(dialog: MagiskDialog) {
        with(dialog) {
            val view = LayoutInflater.from(context).inflate(R.layout.markdown_window_md2, null)
            applyView(view)
            (ownerActivity as BaseActivity).lifecycleScope.launch {
                val tv = view.findViewById<TextView>(R.id.md_txt)
                withContext(Dispatchers.IO) {
                    try {
                        ServiceLocator.markwon.setMarkdown(tv, getMarkdownText())
                    } catch (e: Exception) {
                        if (e is CancellationException)
                            throw e
                        Timber.e(e)
                        tv.post { tv.setText(R.string.download_file_error) }
                    }
                }
            }
        }
    }
}
