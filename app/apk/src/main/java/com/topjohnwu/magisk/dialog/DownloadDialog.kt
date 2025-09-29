package com.topjohnwu.magisk.dialog

import android.net.Uri
import android.text.InputType
import android.widget.EditText
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.events.DialogBuilder
import com.topjohnwu.magisk.view.MagiskDialog

class DownloadDialog(private val callback: (Uri) -> Unit) : DialogBuilder {

    override fun build(dialog: MagiskDialog) {
        val editText = EditText(dialog.context).apply {
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI
            hint = context.getString(R.string.download_dialog_msg)
            requestFocus()
        }

        dialog.apply {
            setTitle(R.string.download_dialog_title)
            setView(editText)
            setButton(MagiskDialog.ButtonType.POSITIVE) {
                text = android.R.string.ok
                onClick {
                    val url = editText.text.toString().trim()
                    isValidUrl(url)?.let {
                        doNotDismiss = false
                        callback(it)
                    } ?: run {
                        doNotDismiss = true
                        editText.error = context.getString(R.string.download_dialog_title)
                    }
                }
            }
            setButton(MagiskDialog.ButtonType.NEGATIVE) {
                text = android.R.string.cancel
            }
            setCancelable(true)
        }
    }

    private fun isValidUrl(url: String): Uri? {
        if (url.isEmpty()) {
            return null
        }
        val uri = url.toUri()
        if (!uri.scheme.equals("https", ignoreCase = true)) {
            return null
        }
        if (uri.host.isNullOrEmpty()) {
            return null
        }
        if (uri.path.isNullOrEmpty()) {
            return null
        }
        return uri
    }
}
