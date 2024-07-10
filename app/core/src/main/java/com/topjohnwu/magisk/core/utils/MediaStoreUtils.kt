package com.topjohnwu.magisk.core.utils

import android.content.ContentUris
import android.content.ContentValues
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import android.provider.OpenableColumns
import androidx.annotation.RequiresApi
import androidx.core.net.toFile
import androidx.core.net.toUri
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Config
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

@Suppress("DEPRECATION")
object MediaStoreUtils {

    private val cr get() = AppContext.contentResolver

    private fun relativePath(name: String) =
        if (name.isEmpty()) Environment.DIRECTORY_DOWNLOADS
        else Environment.DIRECTORY_DOWNLOADS + File.separator + name

    fun fullPath(name: String): String =
        File(Environment.getExternalStorageDirectory(), relativePath(name)).canonicalPath

    private val downloadPath get() = relativePath(Config.downloadDir)

    @RequiresApi(api = 30)
    @Throws(IOException::class)
    private fun insertFile(displayName: String): MediaStoreFile {
        val values = ContentValues()
        values.put(MediaStore.MediaColumns.RELATIVE_PATH, downloadPath)
        values.put(MediaStore.MediaColumns.DISPLAY_NAME, displayName)

        // When a file with the same name exists and was not created by us:
        // - Before Android 11, insert will return null
        // - On Android 11+, the system will automatically create a new name
        // Thus the reason to restrict this method call to API 30+
        val fileUri = cr.insert(MediaStore.Downloads.EXTERNAL_CONTENT_URI, values)
            ?: throw IOException("Can't insert $displayName.")

        val projection = arrayOf(MediaStore.MediaColumns._ID, MediaStore.MediaColumns.DATA)
        cr.query(fileUri, projection, null, null, null)?.use { cursor ->
            val idIndex = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns._ID)
            val dataColumn = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA)
            if (cursor.moveToFirst()) {
                val id = cursor.getLong(idIndex)
                val data = cursor.getString(dataColumn)
                return MediaStoreFile(id, data)
            }
        }

        throw IOException("Can't insert $displayName.")
    }

    @RequiresApi(api = 29)
    private fun queryFile(displayName: String): UriFile? {
        val projection = arrayOf(MediaStore.MediaColumns._ID, MediaStore.MediaColumns.DATA)
        // Before Android 10, we wrote the DISPLAY_NAME field when insert, so it can be used.
        val selection = "${MediaStore.MediaColumns.DISPLAY_NAME} == ?"
        val selectionArgs = arrayOf(displayName)
        val sortOrder = "${MediaStore.MediaColumns.DATE_ADDED} DESC"
        val query = cr.query(
            MediaStore.Downloads.EXTERNAL_CONTENT_URI,
            projection, selection, selectionArgs, sortOrder)
        query?.use { cursor ->
            val idColumn = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns._ID)
            val dataColumn = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA)
            while (cursor.moveToNext()) {
                val id = cursor.getLong(idColumn)
                val data = cursor.getString(dataColumn)
                if (data.endsWith(downloadPath + File.separator + displayName)) {
                    return MediaStoreFile(id, data)
                }
            }
        }
        return null
    }

    @Throws(IOException::class)
    fun getFile(displayName: String): UriFile {
        return if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            // Fallback to file based I/O pre Android 11
            val parent = File(Environment.getExternalStorageDirectory(), downloadPath)
            parent.mkdirs()
            LegacyUriFile(File(parent, displayName))
        } else {
            queryFile(displayName) ?: insertFile(displayName)
        }
    }

    fun Uri.inputStream() = cr.openInputStream(this) ?: throw FileNotFoundException()

    fun Uri.outputStream() = cr.openOutputStream(this, "rwt") ?: throw FileNotFoundException()

    val Uri.displayName: String get() {
        if (scheme == "file") {
            // Simple uri wrapper over file, directly get file name
            return toFile().name
        }
        require(scheme == "content") { "Uri lacks 'content' scheme: $this" }
        val projection = arrayOf(OpenableColumns.DISPLAY_NAME)
        cr.query(this, projection, null, null, null)?.use { cursor ->
            val displayNameColumn = cursor.getColumnIndexOrThrow(OpenableColumns.DISPLAY_NAME)
            if (cursor.moveToFirst()) {
                return cursor.getString(displayNameColumn)
            }
        }
        return this.toString()
    }

    interface UriFile {
        val uri: Uri
        fun delete(): Boolean
    }

    private class LegacyUriFile(private val file: File) : UriFile {
        override val uri = file.toUri()
        override fun delete() = file.delete()
        override fun toString() = file.toString()
    }

    @RequiresApi(api = 29)
    private class MediaStoreFile(private val id: Long, private val data: String) : UriFile {
        override val uri = ContentUris.withAppendedId(MediaStore.Downloads.EXTERNAL_CONTENT_URI, id)
        override fun toString() = data
        override fun delete(): Boolean {
            val selection = "${MediaStore.MediaColumns._ID} == ?"
            val selectionArgs = arrayOf(id.toString())
            return cr.delete(uri, selection, selectionArgs) == 1
        }
    }
}
