package com.topjohnwu.magisk.utils

import android.annotation.SuppressLint
import android.content.ContentResolver
import android.content.ContentUris
import android.content.ContentValues
import android.content.Context
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import android.provider.OpenableColumns
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ktx.get
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException
import java.io.OutputStream
import java.security.MessageDigest
import kotlin.experimental.and

@Suppress("DEPRECATION")
object MediaStoreUtils {

    private val cr: ContentResolver by lazy { get<Context>().contentResolver }

    @SuppressLint("InlinedApi")
    private val tableUri: Uri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
        MediaStore.Downloads.EXTERNAL_CONTENT_URI
    } else {
        MediaStore.Files.getContentUri(MediaStore.VOLUME_EXTERNAL)
    }

    fun relativePath(appName: String): String {
        var path = Environment.DIRECTORY_DOWNLOADS
        if (appName.isNotEmpty()) {
            path += File.separator + appName
        }
        return path
    }

    @Throws(IOException::class)
    private fun insertFile(displayName: String): MediaStoreFile {
        val values = ContentValues()
        val relativePath = relativePath(Config.downloadPath)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            values.put(MediaStore.MediaColumns.RELATIVE_PATH, relativePath)
        } else {
            val parent = File(Environment.getExternalStorageDirectory(), relativePath)
            values.put(MediaStore.MediaColumns.DATA, File(parent, displayName).path)
            parent.mkdirs()
        }
        values.put(MediaStore.MediaColumns.DISPLAY_NAME, displayName)

        // before Android 11, MediaStore can not rename new file when file exists,
        // insert will return null. use newFile() instead.
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

    private fun queryFile(displayName: String): MediaStoreFile? {
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
                val relativePath = relativePath(Config.downloadPath)
                if (data.endsWith(relativePath + File.separator + displayName)) {
                    return MediaStoreFile(id, data)
                }
            }
        }
        return null
    }

    @Throws(IOException::class)
    fun newFile(displayName: String): MediaStoreFile {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            insertFile(displayName)
        } else {
            queryFile(displayName)?.delete()
            insertFile(displayName)
        }
    }

    @Throws(IOException::class)
    fun getFile(displayName: String): MediaStoreFile {
        return queryFile(displayName) ?: insertFile(displayName)
    }

    fun Uri.inputStream() = cr.openInputStream(this) ?: throw FileNotFoundException()

    fun Uri.outputStream() = cr.openOutputStream(this) ?: throw FileNotFoundException()

    fun Uri.getDisplayName(): String {
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

    data class MediaStoreFile(val id: Long, private val data: String) {
        val uri: Uri = ContentUris.withAppendedId(tableUri, id)
        override fun toString() = data

        fun delete(): Boolean {
            val selection = "${MediaStore.MediaColumns._ID} == ?"
            val selectionArgs = arrayOf(id.toString())
            return cr.delete(uri, selection, selectionArgs) == 1
        }
    }
}
