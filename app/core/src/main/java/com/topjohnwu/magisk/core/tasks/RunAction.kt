package com.topjohnwu.magisk.core.tasks

import android.net.Uri
import androidx.core.net.toFile
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.ktx.writeTo
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.displayName
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.inputStream
import com.topjohnwu.magisk.core.utils.unzip
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException

open class RunAction(
    private val module: String,
    private val console: MutableList<String>,
    private val logs: MutableList<String>
) {
    @Throws(IOException::class)
    private suspend fun run(): Boolean {
        return Shell.cmd("run_action \'$module\'").to(console, logs).exec().isSuccess
    }

    open suspend fun exec() = withContext(Dispatchers.IO) {
        try {
            if (!run()) {
                console.add("! Run action failed")
                false
            } else {
                true
            }
        } catch (e: IOException) {
            Timber.e(e)
            false
        }
    }
}
