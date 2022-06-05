package com.topjohnwu.magisk.core

import android.net.Uri
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.os.ParcelFileDescriptor.MODE_READ_ONLY
import com.topjohnwu.magisk.core.base.BaseProvider
import com.topjohnwu.magisk.core.su.SuCallbackHandler

class Provider : BaseProvider() {

    override fun call(method: String, arg: String?, extras: Bundle?): Bundle? {
        SuCallbackHandler.run(context!!, method, extras)
        return Bundle.EMPTY
    }

    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor? {
        return when (uri.encodedPath ?: return null) {
            "/prefs_file" -> ParcelFileDescriptor.open(Config.prefsFile, MODE_READ_ONLY)
            else -> super.openFile(uri, mode)
        }
    }

    companion object {
        fun preferencesUri(pkg: String): Uri =
            Uri.Builder().scheme("content").authority("$pkg.provider").path("prefs_file").build()
    }
}
