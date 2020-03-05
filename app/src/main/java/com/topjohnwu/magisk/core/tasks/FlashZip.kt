package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.net.Uri
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.utils.unzip
import com.topjohnwu.magisk.extensions.fileName
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.readUri
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.superuser.Shell
import io.reactivex.Single
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

abstract class FlashZip(
    private val mUri: Uri,
    private val console: MutableList<String>,
    private val logs: MutableList<String>
) : FlashResultListener {

    private val context: Context by inject()
    private val installFolder = File(context.cacheDir, "flash").apply {
        if (!exists()) mkdirs()
    }
    private val tmpFile: File = File(installFolder, "install.zip")

    @Throws(IOException::class)
    private fun unzipAndCheck(): Boolean {
        val parentFile = tmpFile.parentFile ?: return false
        tmpFile.unzip(parentFile, "META-INF/com/google/android", true)

        val updaterScript = File(parentFile, "updater-script")
        return Shell
            .su("grep -q '#MAGISK' $updaterScript")
            .exec()
            .isSuccess
    }

    @Throws(IOException::class)
    private fun flash(): Boolean {
        console.add("- Copying zip to temp directory")

        runCatching {
            context.readUri(mUri).use { input ->
                tmpFile.outputStream().use { out -> input.copyTo(out) }
            }
        }.getOrElse {
            when (it) {
                is FileNotFoundException -> console.add("! Invalid Uri")
                is IOException -> console.add("! Cannot copy to cache")
            }
            throw it
        }

        val isMagiskModule = runCatching {
            unzipAndCheck()
        }.getOrElse {
            console.add("! Unzip error")
            throw it
        }

        if (!isMagiskModule) {
            console.add("! This zip is not a Magisk Module!")
            return false
        }

        console.add("- Installing ${mUri.fileName}")

        val parentFile = tmpFile.parent ?: return false

        return Shell
            .su(
                "cd $parentFile",
                "BOOTMODE=true sh update-binary dummy 1 $tmpFile"
            )
            .to(console, logs)
            .exec().isSuccess
    }

    fun exec() = Single
        .fromCallable {
            runCatching {
                flash()
            }.getOrElse {
                it.printStackTrace()
                false
            }.apply {
                Shell.su("cd /", "rm -rf ${tmpFile.parent} ${Const.TMP_FOLDER_PATH}")
                    .submit()
            }
        }
        .subscribeK(onError = { onResult(false) }) { onResult(it) }
        .let { Unit } // ignores result disposable

}
