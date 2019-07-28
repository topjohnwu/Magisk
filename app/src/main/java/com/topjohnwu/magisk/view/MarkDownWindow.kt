package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import com.skoumal.teanity.extensions.subscribeK
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.extensions.inject
import io.reactivex.Single
import ru.noties.markwon.Markwon
import ru.noties.markwon.html.HtmlPlugin
import ru.noties.markwon.image.ImagesPlugin
import ru.noties.markwon.image.svg.SvgPlugin
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
        content.subscribeK {
            val markwon = Markwon.builder(activity)
                    .usePlugin(HtmlPlugin.create())
                    .usePlugin(ImagesPlugin.create(activity))
                    .usePlugin(SvgPlugin.create(activity.resources))
                    .build()
            val alert = AlertDialog.Builder(activity)
            alert.setTitle(title)
            val mv = LayoutInflater.from(activity).inflate(R.layout.markdown_window, null)
            val tv = mv.findViewById<TextView>(R.id.md_txt)
            try {
                markwon.setMarkdown(tv, it)
            } catch (e: ExceptionInInitializerError) {
                //Nothing we can do about this error other than show error message
                tv.setText(R.string.download_file_error)
            }

            alert.setView(mv)
            alert.setNegativeButton(R.string.close) { dialog, _ -> dialog.dismiss() }
            alert.show()
        }
    }
}
