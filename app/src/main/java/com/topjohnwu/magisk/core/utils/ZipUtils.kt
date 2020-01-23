package com.topjohnwu.magisk.core.utils

import com.topjohnwu.superuser.io.SuFile
import com.topjohnwu.superuser.io.SuFileOutputStream
import java.io.File
import java.io.IOException
import java.io.InputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

@Throws(IOException::class)
fun File.unzip(folder: File, path: String = "", junkPath: Boolean = false) {
    inputStream().buffered().use {
        it.unzip(folder, path, junkPath)
    }
}

@Throws(IOException::class)
fun InputStream.unzip(folder: File, path: String, junkPath: Boolean) {
    try {
        val zin = ZipInputStream(this)
        var entry: ZipEntry
        while (true) {
            entry = zin.nextEntry ?: break
            if (!entry.name.startsWith(path) || entry.isDirectory) {
                // Ignore directories, only create files
                continue
            }
            val name = if (junkPath)
                entry.name.substring(entry.name.lastIndexOf('/') + 1)
            else
                entry.name

            var dest = File(folder, name)
            if (!dest.parentFile!!.exists() && !dest.parentFile!!.mkdirs()) {
                dest = SuFile(folder, name)
                dest.parentFile!!.mkdirs()
            }
            SuFileOutputStream(dest).use { out -> zin.copyTo(out) }
        }
    } catch (e: IOException) {
        e.printStackTrace()
        throw e
    }
}
