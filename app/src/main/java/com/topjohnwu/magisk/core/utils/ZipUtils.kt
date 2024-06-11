package com.topjohnwu.magisk.core.utils

import com.topjohnwu.magisk.core.ktx.copyAll
import org.apache.commons.compress.archivers.zip.ZipFile
import java.io.File
import java.io.IOException

@Throws(IOException::class)
suspend fun File.unzip(folder: File, path: String = "", junkPath: Boolean = false) {
    try {
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
                dest.parentFile!!.let {
                    if (!it.exists())
                        it.mkdirs()
                }
                dest.outputStream().use { out -> zip.getInputStream(entry).copyAll(out) }
            }
        }
    } catch (e: IllegalArgumentException) {
        throw IOException(e)
    }
}
