package com.topjohnwu.magisk.core

import android.net.Uri
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.os.ParcelFileDescriptor.MODE_READ_ONLY
import com.topjohnwu.magisk.core.base.BaseProvider
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.TestHandler

class Provider : BaseProvider() {

    override fun call(method: String, arg: String?, extras: Bundle?): Bundle? {
        return when (method) {
            SuCallbackHandler.LOG, SuCallbackHandler.NOTIFY -> {
                SuCallbackHandler.run(context!!, method, extras)
                Bundle.EMPTY
            }
            else -> TestHandler.run(method)
        }
    }

    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor? {
        return when (uri.encodedPath ?: return null) {
            "/prefs_file" -> ParcelFileDescriptor.open(Config.getPrefsFile(), MODE_READ_ONLY)
            else -> super.openFile(uri, mode)
        }
    }

    companion object {
        fun preferencesUri(pkg: String): Uri =
            Uri.Builder().scheme("content").authority("$pkg.provider").path("prefs_file").build()
    }
}
