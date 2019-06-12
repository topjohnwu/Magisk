package com.topjohnwu.magisk.view

import android.content.Context
import android.view.LayoutInflater
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import com.topjohnwu.magisk.R
import com.topjohnwu.net.Networking
import com.topjohnwu.net.ResponseListener
import ru.noties.markwon.Markwon
import ru.noties.markwon.html.HtmlPlugin
import ru.noties.markwon.image.ImagesPlugin
import ru.noties.markwon.image.svg.SvgPlugin
import java.io.InputStream
import java.util.*

object MarkDownWindow {

    fun show(activity: Context, title: String?, url: String) {
        Networking.get(url).getAsString(Listener(activity, title))
    }

    fun show(activity: Context, title: String?, input: InputStream) {
        Scanner(input, "UTF-8").use {
            it.useDelimiter("\\A")
            Listener(activity, title).onResponse(it.next())
        }
    }

    internal class Listener(var activity: Context, var title: String?) : ResponseListener<String> {

        override fun onResponse(md: String) {
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
                markwon.setMarkdown(tv, md)
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
