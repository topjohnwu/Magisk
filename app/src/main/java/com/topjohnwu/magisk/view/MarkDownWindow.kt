package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.extensions.inject
import io.reactivex.Completable
import io.reactivex.Single
import ru.noties.markwon.Markwon
import ru.noties.markwon.html.HtmlPlugin
import ru.noties.markwon.image.ImagesPlugin
import ru.noties.markwon.image.svg.SvgPlugin
import timber.log.Timber
import java.io.InputStream
import java.util.*

object MarkDownWindow {

    private val stringRepo: StringRepository by inject()

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
        val markwon = Markwon.builder(activity)
                .usePlugin(HtmlPlugin.create())
                .usePlugin(ImagesPlugin.create(activity))
                .usePlugin(SvgPlugin.create(activity.resources))
                .build()
        val mv = LayoutInflater.from(activity).inflate(R.layout.markdown_window, null)
        val tv = mv.findViewById<TextView>(R.id.md_txt)

        content.map {
            runCatching {
                markwon.setMarkdown(tv, it)
            }.onFailure {
                Timber.e(it)
                // Always wrap the actual exception as it could be ExceptionInInitializerError,
                // which is a fatal error and RxJava will send it to the global handler and crash
                throw MarkwonException(it)
            }
        }.ignoreElement().onErrorResumeNext {
            // Nothing we can actually do other than show error message
            tv.setText(R.string.download_file_error)
            Completable.complete()
        }.subscribeK {
            AlertDialog.Builder(activity)
                    .setTitle(title)
                    .setView(mv)
                    .setNegativeButton(R.string.close) { dialog, _ -> dialog.dismiss() }
                    .show()
        }
    }

    class MarkwonException(e: Throwable): Exception(e)
}
