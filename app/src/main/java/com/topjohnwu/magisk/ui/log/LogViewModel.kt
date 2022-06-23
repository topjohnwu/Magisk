package com.topjohnwu.magisk.ui.log

import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.databinding.bindExtra
import com.topjohnwu.magisk.databinding.diffListOf
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.ktx.timeFormatStandard
import com.topjohnwu.magisk.ktx.toTime
import com.topjohnwu.magisk.view.TextItem
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.FileInputStream

class LogViewModel(
    private val repo: LogRepository
) : AsyncLoadViewModel() {

    // --- empty view

    val itemEmpty = TextItem(R.string.log_data_none)
    val itemMagiskEmpty = TextItem(R.string.log_data_magisk_none)

    // --- su log

    val items = diffListOf<LogRvItem>()
    val extraBindings = bindExtra {
        it.put(BR.viewModel, this)
    }

    // --- magisk log
    @get:Bindable
    var consoleText = " "
        set(value) = set(value, field, { field = it }, BR.consoleText)

    override suspend fun doLoadWork() {
        consoleText = repo.fetchMagiskLogs()
        val (suLogs, diff) = withContext(Dispatchers.Default) {
            val suLogs = repo.fetchSuLogs().map { LogRvItem(it) }
            suLogs to items.calculateDiff(suLogs)
        }
        items.firstOrNull()?.isTop = false
        items.lastOrNull()?.isBottom = false
        items.update(suLogs, diff)
        items.firstOrNull()?.isTop = true
        items.lastOrNull()?.isBottom = true
    }

    fun saveMagiskLog() = withExternalRW {
        viewModelScope.launch(Dispatchers.IO) {
            val filename = "magisk_log_%s.log".format(
                System.currentTimeMillis().toTime(timeFormatStandard))
            val logFile = MediaStoreUtils.getFile(filename, true)
            logFile.uri.outputStream().bufferedWriter().use { file ->
                file.write("---Detected Device Info---\n\n")
                file.write("isAB=${Info.isAB}\n")
                file.write("isSAR=${Info.isSAR}\n")
                file.write("ramdisk=${Info.ramdisk}\n")

                file.write("\n\n---System Properties---\n\n")
                ProcessBuilder("getprop").start()
                    .inputStream.reader().use { it.copyTo(file) }

                file.write("\n\n---System MountInfo---\n\n")
                FileInputStream("/proc/self/mountinfo").reader().use { it.copyTo(file) }

                file.write("\n---Magisk Logs---\n")
                file.write("${Info.env.versionString} (${Info.env.versionCode})\n\n")
                file.write(consoleText)

                file.write("\n---Manager Logs---\n")
                file.write("${BuildConfig.VERSION_NAME} (${BuildConfig.VERSION_CODE})\n\n")
                ProcessBuilder("logcat", "-d").start()
                    .inputStream.reader().use { it.copyTo(file) }
            }
            SnackbarEvent(logFile.toString()).publish()
        }
    }

    fun clearMagiskLog() = repo.clearMagiskLogs {
        SnackbarEvent(R.string.logs_cleared).publish()
        startLoading()
    }

    fun clearLog() = viewModelScope.launch {
        repo.clearLogs()
        SnackbarEvent(R.string.logs_cleared).publish()
        startLoading()
    }
}
