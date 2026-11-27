package com.topjohnwu.magisk.wear.files

import android.content.Context
import android.net.Uri
import com.topjohnwu.magisk.core.ktx.await
import com.topjohnwu.magisk.terminal.shellQuoted
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.IOException
import java.util.Locale

/**
 * Files read through the root shell, for devices where no app handles ACTION_OPEN_DOCUMENT
 * (e.g. most Wear OS watches). Users put files there with `adb push`.
 */
object RootFiles {

    const val DOWNLOAD_DIR = "/sdcard/Download"

    // Copies of picked files, so the app can open them without storage permissions
    private const val CACHE_DIR = "picked"

    /**
     * Keep the names in [output] that end with an extension in [ext] (ignoring case), sorted
     * by name ignoring case. Empty lines are dropped.
     */
    fun parseListing(output: List<String>, ext: Set<String>): List<String> {
        val extensions = ext.mapTo(HashSet()) { it.lowercase(Locale.ROOT) }
        return output
            .filter { it.isNotEmpty() }
            .filter { name ->
                val lower = name.lowercase(Locale.ROOT)
                extensions.any { lower.endsWith(".$it") }
            }
            .sortedWith(String.CASE_INSENSITIVE_ORDER)
    }

    /**
     * List the regular files in [dir] with an extension in [ext]. Returns an empty list if
     * the directory can't be read, e.g. without root.
     */
    suspend fun list(dir: String = DOWNLOAD_DIR, ext: Set<String>): List<String> {
        // Only regular files: a directory named like "x.zip" can't be picked. printf instead
        // of echo, so names starting with "-" are printed as they are.
        val cmd = "cd ${dir.shellQuoted()} && " +
            "for f in *; do [ -f \"\$f\" ] && printf '%s\\n' \"\$f\"; done"
        // The loop's exit code is that of the last test, so only the output matters
        return parseListing(Shell.cmd(cmd).await().out, ext)
    }

    /**
     * Copy [name] in [dir] to the app's cache with the root shell and return the copy's file
     * URI. Copies of earlier picks are removed first, so large images don't pile up in the
     * cache.
     *
     * The app creates the copy itself and root only writes into it. A file created by root
     * would get the root shell's SELinux level, without the app's categories, so the app
     * could neither open nor delete it. Writing through a redirect truncates the existing
     * file, which keeps the app's owner and label.
     */
    suspend fun copyToCache(
        context: Context,
        name: String,
        dir: String = DOWNLOAD_DIR,
    ): Uri = withContext(Dispatchers.IO) {
        require('/' !in name && name != "." && name != "..") { "Not a file name: $name" }
        val cache = File(context.cacheDir, CACHE_DIR)
        // Every copy was created by the app, so the app can delete it
        cache.deleteRecursively()
        if (!cache.mkdirs()) throw IOException("Failed to create $cache")
        val dest = File(cache, name)
        if (!dest.createNewFile()) throw IOException("Failed to create $dest")
        val src = File(dir, name).path
        val result = Shell.cmd("cat ${src.shellQuoted()} > ${dest.path.shellQuoted()}").exec()
        if (!result.isSuccess) {
            dest.delete()
            throw IOException("Failed to copy $src: ${result.err.joinToString("\n")}")
        }
        Uri.fromFile(dest)
    }
}
