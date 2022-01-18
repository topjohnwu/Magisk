package com.topjohnwu.magisk.core

import android.content.ContentProvider
import android.content.ContentValues
import android.content.Context
import android.content.pm.ProviderInfo
import android.database.Cursor
import android.net.Uri
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.os.ParcelFileDescriptor.MODE_READ_ONLY
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import java.io.File

class Provider : ContentProvider() {

    override fun attachInfo(context: Context, info: ProviderInfo) {
        super.attachInfo(context.wrap(), info)
    }

    override fun call(method: String, arg: String?, extras: Bundle?): Bundle? {
        SuCallbackHandler.run(context!!, method, extras)
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

    override fun onCreate() = true
    override fun getType(uri: Uri): String? = null
    override fun insert(uri: Uri, values: ContentValues?): Uri? = null
    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<out String>?) = 0
    override fun update(uri: Uri, values: ContentValues?, selection: String?, selectionArgs: Array<out String>?) = 0
    override fun query(uri: Uri, projection: Array<out String>?, selection: String?, selectionArgs: Array<out String>?, sortOrder: String?): Cursor? = null
}
