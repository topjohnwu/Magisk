package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.net.Uri
import androidx.core.os.postDelayed
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import com.topjohnwu.magisk.core.utils.unzip
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

open class FlashZip(
    private val mUri: Uri,
    private val console: MutableList<String>,
    private val logs: MutableList<String>
): KoinComponent {

    val context: Context by inject()
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
            mUri.inputStream().writeTo(tmpFile)
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

        console.add("- Installing ${mUri.displayName}")

        val parentFile = tmpFile.parent ?: return false

        return Shell
            .su(
                "cd $parentFile",
                "BOOTMODE=true sh update-binary dummy 1 $tmpFile"
            )
            .to(console, logs)
            .exec().isSuccess
    }

    open suspend fun exec() = withContext(Dispatchers.IO) {
        try {
            if (!flash()) {
                console.add("! Installation failed")
                false
            } else {
                true
            }
        } catch (e: IOException) {
            Timber.e(e)
            false
        } finally {
            Shell.su("cd /", "rm -rf ${tmpFile.parent} ${Const.TMP_FOLDER_PATH}").submit()
        }
    }

    class Uninstall(
        uri: Uri,
        console: MutableList<String>,
        log: MutableList<String>
    ) : FlashZip(uri, console, log) {

        override suspend fun exec(): Boolean {
            val success = super.exec()
            if (success) {
                UiThreadHandler.handler.postDelayed(3000) {
                    Shell.su("pm uninstall " + context.packageName).exec()
                }
            }
            return success
        }
    }

}
