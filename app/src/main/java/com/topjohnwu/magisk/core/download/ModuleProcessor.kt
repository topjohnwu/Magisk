package com.topjohnwu.magisk.core.download

import com.topjohnwu.magisk.extensions.withStreams
import java.io.File
import java.io.InputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

fun InputStream.toModule(file: File, installer: InputStream) {

    val input = ZipInputStream(buffered())
    val output = ZipOutputStream(file.outputStream().buffered())

    withStreams(input, output) { zin, zout ->
        zout.putNextEntry(ZipEntry("META-INF/"))
        zout.putNextEntry(ZipEntry("META-INF/com/"))
        zout.putNextEntry(ZipEntry("META-INF/com/google/"))
        zout.putNextEntry(ZipEntry("META-INF/com/google/android/"))
        zout.putNextEntry(ZipEntry("META-INF/com/google/android/update-binary"))
        installer.copyTo(zout)

        zout.putNextEntry(ZipEntry("META-INF/com/google/android/updater-script"))
        zout.write("#MAGISK\n".toByteArray(charset("UTF-8")))

        var off = -1
        var entry: ZipEntry? = zin.nextEntry
        while (entry != null) {
            if (off < 0) {
                off = entry.name.indexOf('/') + 1
            }

            val path = entry.name.substring(off)
            if (path.isNotEmpty() && !path.startsWith("META-INF")) {
                zout.putNextEntry(ZipEntry(path))
                if (!entry.isDirectory) {
                    zin.copyTo(zout)
                }
            }

            entry = zin.nextEntry
        }
    }
}
