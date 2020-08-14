package com.topjohnwu.magisk.databinding

import android.view.View
import android.widget.TextView
import androidx.core.view.isGone
import androidx.core.view.isInvisible
import androidx.databinding.BindingAdapter
import com.topjohnwu.magisk.ktx.coroutineScope
import com.topjohnwu.magisk.ktx.get
import io.noties.markwon.Markwon
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

@BindingAdapter("gone")
fun setGone(view: View, gone: Boolean) {
    view.isGone = gone
}

@BindingAdapter("invisible")
fun setInvisible(view: View, invisible: Boolean) {
    view.isInvisible = invisible
}

@BindingAdapter("goneUnless")
fun setGoneUnless(view: View, goneUnless: Boolean) {
    setGone(view, goneUnless.not())
}

@BindingAdapter("invisibleUnless")
fun setInvisibleUnless(view: View, invisibleUnless: Boolean) {
    setInvisible(view, invisibleUnless.not())
}

@BindingAdapter("markdownText")
fun setMarkdownText(tv: TextView, text: CharSequence) {
    tv.coroutineScope.launch(Dispatchers.IO) {
        val markwon = get<Markwon>()
        markwon.setMarkdown(tv, text.toString())
    }
}
