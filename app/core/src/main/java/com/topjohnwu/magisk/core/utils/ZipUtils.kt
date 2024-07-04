package com.topjohnwu.magisk.core.utils

import com.topjohnwu.magisk.core.ktx.copyAll
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipArchiveInputStream
import org.apache.commons.compress.archivers.zip.ZipFile
import java.io.File
import java.io.IOException
import java.io.InputStream

@Throws(IOException::class)
suspend fun File.unzip(folder: File, path: String = "", junkPath: Boolean = false) {
    ZipFile.Builder().setFile(this).get().use { zip ->
        for (entry in zip.entries) {
            if (!entry.name.startsWith(path) || entry.isDirectory) {
                // Ignore directories, only create files
                continue
            }
            val name = if (junkPath)
                entry.name.substring(entry.name.lastIndexOf('/') + 1)
            else
                entry.name
            val dest = File(folder, name)
            dest.parentFile?.mkdirs()
            dest.outputStream().use { out -> zip.getInputStream(entry).copyAll(out) }
        }
    }
}

@Throws(IOException::class)
suspend fun InputStream.unzip(folder: File, path: String = "", junkPath: Boolean = false) {
    ZipArchiveInputStream(this).use { zin ->
        var entry: ZipArchiveEntry
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
            dest.parentFile?.mkdirs()
            dest.outputStream().use { out -> zin.copyAll(out) }
        }
    }
}
