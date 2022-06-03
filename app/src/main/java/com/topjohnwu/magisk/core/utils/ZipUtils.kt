package com.topjohnwu.magisk.core.utils

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

            val dest = File(folder, name)
            dest.parentFile!!.let {
                if (!it.exists())
                    it.mkdirs()
            }
            dest.outputStream().use { out -> zin.copyTo(out) }
        }
    } catch (e: IllegalArgumentException) {
        throw IOException(e)
    }
}
