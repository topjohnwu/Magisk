package com.topjohnwu.magisk.core

import android.content.Context
import android.content.pm.ProviderInfo
import android.net.Uri
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.os.ParcelFileDescriptor.MODE_READ_ONLY
import com.topjohnwu.magisk.FileProvider
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import java.io.File

open class Provider : FileProvider() {

    override fun attachInfo(context: Context, info: ProviderInfo?) {
        super.attachInfo(context.wrap(), info)
    }

    override fun call(method: String, arg: String?, extras: Bundle?): Bundle? {
        SuCallbackHandler(context!!, method, extras)
        return Bundle.EMPTY
    }

    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor? {
        return when (uri.encodedPath ?: return null) {
            "/apk_file" -> ParcelFileDescriptor.open(File(context!!.packageCodePath), MODE_READ_ONLY)
            "/prefs_file" -> ParcelFileDescriptor.open(Config.prefsFile, MODE_READ_ONLY)
            else -> super.openFile(uri, mode)
        }
    }

    companion object {
        fun APK_URI(pkg: String) =
            Uri.Builder().scheme("content").authority("$pkg.provider").path("apk_file").build()

        fun PREFS_URI(pkg: String) =
            Uri.Builder().scheme("content").authority("$pkg.provider").path("prefs_file").build()
    }
}
