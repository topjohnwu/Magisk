package com.topjohnwu.magisk.model.download

import android.content.Context
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.utils.cachedFile
import com.topjohnwu.magisk.utils.withStreams
import java.io.File
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

class ModuleTransformer(
    private val context: Context,
    subject: DownloadSubject
) {

    private val destination = context.cachedFile(subject.fileName)

    fun inject(file: File, installer: File): File {
        return injectInternal(move(file), installer)
    }

    // ---

    private fun injectInternal(file: File, installer: File): File {
        val input = ZipInputStream(file.inputStream())
        val output = ZipOutputStream(destination.outputStream())

        withStreams(input, output) { zin, zout ->
            zout.putNextEntry(ZipEntry("META-INF/"))
            zout.putNextEntry(ZipEntry("META-INF/com/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/android/"))
            zout.putNextEntry(ZipEntry("META-INF/com/google/android/update-binary"))
            installer.inputStream().copyTo(zout).also { zout.flush() }

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
                        zin.copyTo(zout).also { zout.flush() }
                    }
                }

                entry = zin.nextEntry
            }
        }

        file.delete()

        return destination
    }

    private fun move(file: File) = context.cachedFile("temp").apply {
        file.renameTo(this)
    }

}