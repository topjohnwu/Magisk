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
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.di.AppContext
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException
import java.io.OutputStream
import java.security.MessageDigest
import kotlin.experimental.and

@Suppress("DEPRECATION")
object MediaStoreUtils {

    private val cr get() = AppContext.contentResolver

    @get:RequiresApi(api = 29)
    private val tableUri
        get() = MediaStore.Downloads.EXTERNAL_CONTENT_URI

    private fun relativePath(name: String) =
        if (name.isEmpty()) Environment.DIRECTORY_DOWNLOADS
        else Environment.DIRECTORY_DOWNLOADS + File.separator + name

    fun fullPath(name: String): String =
        File(Environment.getExternalStorageDirectory(), relativePath(name)).canonicalPath

    private val relativePath get() = relativePath(Config.downloadDir)

    @RequiresApi(api = 30)
    @Throws(IOException::class)
    private fun insertFile(displayName: String): MediaStoreFile {
        val values = ContentValues()
        values.put(MediaStore.MediaColumns.RELATIVE_PATH, relativePath)
        values.put(MediaStore.MediaColumns.DISPLAY_NAME, displayName)

        // When a file with the same name exists and was not created by us:
        // - Before Android 11, insert will return null
        // - On Android 11+, the system will automatically create a new name
        // Thus the reason to restrict this method call to API 30+
        val fileUri = cr.insert(tableUri, values) ?: throw IOException("Can't insert $displayName.")

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
        cr.query(tableUri, projection, selection, selectionArgs, sortOrder)?.use { cursor ->
            val idColumn = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns._ID)
            val dataColumn = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA)
            while (cursor.moveToNext()) {
                val id = cursor.getLong(idColumn)
                val data = cursor.getString(dataColumn)
                if (data.endsWith(relativePath + File.separator + displayName)) {
                    return MediaStoreFile(id, data)
                }
            }
        }
        return null
    }

    @Throws(IOException::class)
    fun getFile(displayName: String, skipQuery: Boolean = false): UriFile {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            // Fallback to file based I/O pre Android 11
            val parent = File(Environment.getExternalStorageDirectory(), relativePath)
            parent.mkdirs()
            return LegacyUriFile(File(parent, displayName))
        }

        return if (skipQuery) insertFile(displayName)
        else queryFile(displayName) ?: insertFile(displayName)
    }

    fun Uri.inputStream() = cr.openInputStream(this) ?: throw FileNotFoundException()

    fun Uri.outputStream() = cr.openOutputStream(this, "rwt") ?: throw FileNotFoundException()

    fun Uri.fileDescriptor(mode: String) = cr.openFileDescriptor(this, mode) ?: throw FileNotFoundException()

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

    fun Uri.checkSum(alg: String, reference: String) = runCatching {
        this.inputStream().use {
            val digest = MessageDigest.getInstance(alg)
            it.copyTo(object : OutputStream() {
                override fun write(b: Int) {
                    digest.update(b.toByte())
                }

                override fun write(b: ByteArray, off: Int, len: Int) {
                    digest.update(b, off, len)
                }
            })
            val sb = StringBuilder()
            digest.digest().forEach { b -> sb.append("%02x".format(b and 0xff.toByte())) }
            sb.toString() == reference
        }
    }.getOrElse { false }

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
        override val uri = ContentUris.withAppendedId(tableUri, id)
        override fun toString() = data
        override fun delete(): Boolean {
            val selection = "${MediaStore.MediaColumns._ID} == ?"
            val selectionArgs = arrayOf(id.toString())
            return cr.delete(uri, selection, selectionArgs) == 1
        }
    }
}
