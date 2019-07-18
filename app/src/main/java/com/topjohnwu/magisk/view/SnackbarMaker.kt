package com.topjohnwu.magisk.view

import android.app.Activity
import android.net.Uri
import android.view.View
import android.widget.TextView
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.fileName

object SnackbarMaker {

    fun make(activity: Activity, text: CharSequence, duration: Int): Snackbar {
        val view = activity.findViewById<View>(android.R.id.content)
        return make(view, text, duration)
    }

    fun make(activity: Activity, @StringRes resId: Int, duration: Int): Snackbar {
        return make(activity, activity.getString(resId), duration)
    }

    fun make(view: View, text: CharSequence, duration: Int): Snackbar {
        val snack = Snackbar.make(view, text, duration)
        setup(snack)
        return snack
    }

    fun make(view: View, @StringRes resId: Int, duration: Int): Snackbar {
        val snack = Snackbar.make(view, resId, duration)
        setup(snack)
        return snack
    }

    private fun setup(snack: Snackbar) {
        val text = snack.view.findViewById<TextView>(com.google.android.material.R.id.snackbar_text)
        text.maxLines = Integer.MAX_VALUE
    }

    fun showUri(activity: Activity, uri: Uri) {
        make(activity, activity.getString(R.string.internal_storage,
                "/Download/" + uri.fileName),
                Snackbar.LENGTH_LONG)
                .setAction(R.string.ok) { }.show()
    }
}
