package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
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
        val mv = LayoutInflater.from(activity).inflate(R.layout.markdown_window, null)
        val tv = mv.findViewById<TextView>(R.id.md_txt)

        content.map {
            markwon.setMarkdown(tv, it)
        }.ignoreElement().onErrorResumeNext {
            // Nothing we can actually do other than show error message
            Timber.e(it)
            tv.setText(R.string.download_file_error)
            Completable.complete()
        }.subscribeK {
            AlertDialog.Builder(activity)
                    .setTitle(title)
                    .setView(mv)
                    .setNegativeButton(android.R.string.cancel) { dialog, _ -> dialog.dismiss() }
                    .show()
        }
    }
}
