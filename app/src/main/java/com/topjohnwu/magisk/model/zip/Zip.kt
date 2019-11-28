package com.topjohnwu.magisk.model.zip

import com.topjohnwu.magisk.extensions.forEach
import com.topjohnwu.magisk.extensions.withStreams
import com.topjohnwu.superuser.io.SuFile
import java.io.File
import java.util.zip.ZipInputStream


class Zip private constructor(private val values: Builder) {

    companion object {
        operator fun invoke(builder: Builder.() -> Unit): Zip {
            return Zip(Builder().apply(builder))
        }
    }

    class Builder {
        lateinit var zip: File
        lateinit var destination: File
        var excludeDirs = true
    }

    data class Path(val path: String, val pullFromDir: Boolean = true)

    fun unzip(vararg paths: Pair<String, Boolean>) =
        unzip(*paths.map { Path(it.first, it.second) }.toTypedArray())

    @Suppress("RedundantLambdaArrow")
    fun unzip(vararg paths: Path) {
        ensureRequiredParams()

        values.zip.zipStream().use {
            it.forEach { e ->
                val currentPath = paths.firstOrNull { e.name.startsWith(it.path) }
                val isDirectory = values.excludeDirs && e.isDirectory
                if (currentPath == null || isDirectory) {
                    // Ignore directories, only create files
                    return@forEach
                }

                val name = if (currentPath.pullFromDir) {
                    e.name.substring(e.name.lastIndexOf('/') + 1)
                } else {
                    e.name
                }

                val out = File(values.destination, name)
                    .ensureExists()
                    .outputStream()
                //.suOutputStream()

                withStreams(it, out) { reader, writer ->
                    reader.copyTo(writer)
                }
            }
        }
    }

    private fun ensureRequiredParams() {
        if (!values.zip.exists()) {
            throw RuntimeException("Zip file does not exist")
        }
    }

    private fun File.ensureExists() =
        if ((!parentFile.exists() && !parentFile.mkdirs()) || parentFile is SuFile) {
            SuFile(parentFile, name).apply { parentFile.mkdirs() }
        } else {
            this
        }

    private fun File.zipStream() = ZipInputStream(inputStream())

}