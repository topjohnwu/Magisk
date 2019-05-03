package com.topjohnwu.magisk.tasks

import android.net.Uri

import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.utils.*
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler

import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

abstract class FlashZip(private val mUri: Uri,
                        private val console: MutableList<String>,
                        private val logs: MutableList<String>) {

    private val tmpFile: File = File(App.self.cacheDir, "install.zip")

    @Throws(IOException::class)
    private fun unzipAndCheck(): Boolean {
        unzip(tmpFile, tmpFile.parentFile!!, "META-INF/com/google/android", true)
        return Shell.su("grep -q '#MAGISK' ${File(tmpFile.parentFile, "updater-script")}")
                .exec().isSuccess
    }

    @Throws(IOException::class)
    private fun flash(): Boolean {
        console.add("- Copying zip to temp directory")
        try {
            App.self.readUri(mUri).use { input ->
                tmpFile.outputStream().use { out -> input.copyTo(out) }
            }
        } catch (e: FileNotFoundException) {
            console.add("! Invalid Uri")
            throw e
        } catch (e: IOException) {
            console.add("! Cannot copy to cache")
            throw e
        }

        try {
            if (!unzipAndCheck()) {
                console.add("! This zip is not a Magisk Module!")
                return false
            }
        } catch (e: IOException) {
            console.add("! Unzip error")
            throw e
        }

        console.add("- Installing ${mUri.fileName}")
        return Shell.su("cd " + tmpFile.parent!!,
                "BOOTMODE=true sh update-binary dummy 1 $tmpFile")
                .to(console, logs)
                .exec().isSuccess
    }

    fun exec() {
        App.THREAD_POOL.execute {
            val success = try {
                flash()
            } catch (e: IOException) {
                false
            }
            Shell.su("cd /", "rm -rf ${tmpFile.parent} ${Const.TMP_FOLDER_PATH}").submit()
            UiThreadHandler.run { onResult(success) }
        }
    }

    protected abstract fun onResult(success: Boolean)
}
