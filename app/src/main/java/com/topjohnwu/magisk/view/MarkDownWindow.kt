package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.extensions.subscribeK
import io.noties.markwon.Markwon
import io.reactivex.Completable
import io.reactivex.Single
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.InputStream
import java.util.*

object MarkDownWindow : KoinComponent {

    private val stringRepo: StringRepository by inject()
    private val markwon: Markwon by inject()

    fun show(activity: Context, title: String?, url: String) {
        show(activity, title, stringRepo.getString(url))
    }

    fun show(activity: Context, title: String?, input: InputStream) {
        Single.just(Scanner(input, "UTF-8").apply { useDelimiter("\\A") })
            .map { it.next() }
            .also {
                show(activity, title, it)
            }
    }

    fun show(activity: Context, title: String?, content: Single<String>) {
        val mdRes = R.layout.markdown_window_md2
        val mv = LayoutInflater.from(activity).inflate(mdRes, null)
        val tv = mv.findViewById<TextView>(R.id.md_txt)

        content.map {
            markwon.setMarkdown(tv, it)
        }.ignoreElement().onErrorResumeNext {
            // Nothing we can actually do other than show error message
            Timber.e(it)
            tv.setText(R.string.download_file_error)
            Completable.complete()
        }.subscribeK {
            MagiskDialog(activity)
                .applyTitle(title ?: "")
                .applyView(mv)
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = android.R.string.cancel
                }
                .reveal()
            return@subscribeK
        }
    }
}
