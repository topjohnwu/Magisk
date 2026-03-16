package com.topjohnwu.magisk.ui.log

import android.system.Os
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.BuildConfig
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.timeFormatStandard
import com.topjohnwu.magisk.core.ktx.toTime
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.core.repository.LogRepository
import com.topjohnwu.magisk.core.su.SuEvents
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.FileInputStream

class LogViewModel(
    private val repo: LogRepository
) : AsyncLoadViewModel() {

    init {
        @OptIn(kotlinx.coroutines.FlowPreview::class)
        viewModelScope.launch {
            SuEvents.logUpdated.debounce(500).collect { reload() }
        }
    }

    data class UiState(
        val loading: Boolean = true,
        val magiskLog: String = "",
        val magiskLogEntries: List<MagiskLogEntry> = emptyList(),
        val suLogs: List<SuLog> = emptyList(),
    )

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    private var magiskLogRaw = ""

    override suspend fun doLoadWork() {
        _uiState.update { it.copy(loading = true) }
        withContext(Dispatchers.Default) {
            magiskLogRaw = repo.fetchMagiskLogs()
            val suLogs = repo.fetchSuLogs()
            val entries = MagiskLogParser.parse(magiskLogRaw)
            _uiState.update { it.copy(
                loading = false,
                magiskLog = magiskLogRaw,
                magiskLogEntries = entries,
                suLogs = suLogs,
            ) }
        }
    }

    fun saveMagiskLog() {
        viewModelScope.launch(Dispatchers.IO) {
            val filename = "magisk_log_%s.log".format(
                System.currentTimeMillis().toTime(timeFormatStandard))
            val logFile = MediaStoreUtils.getFile(filename)
            logFile.uri.outputStream().bufferedWriter().use { file ->
                file.write("---Detected Device Info---\n\n")
                file.write("isAB=${Info.isAB}\n")
                file.write("isSAR=${Info.isSAR}\n")
                file.write("ramdisk=${Info.ramdisk}\n")
                val uname = Os.uname()
                file.write("kernel=${uname.sysname} ${uname.machine} ${uname.release} ${uname.version}\n")

                file.write("\n\n---System Properties---\n\n")
                ProcessBuilder("getprop").start()
                    .inputStream.reader().use { it.copyTo(file) }

                file.write("\n\n---Environment Variables---\n\n")
                System.getenv().forEach { (key, value) -> file.write("${key}=${value}\n") }

                file.write("\n\n---System MountInfo---\n\n")
                FileInputStream("/proc/self/mountinfo").reader().use { it.copyTo(file) }

                file.write("\n---Magisk Logs---\n")
                file.write("${Info.env.versionString} (${Info.env.versionCode})\n\n")
                if (Info.env.isActive) file.write(magiskLogRaw)

                file.write("\n---Manager Logs---\n")
                file.write("${BuildConfig.APP_VERSION_NAME} (${BuildConfig.APP_VERSION_CODE})\n\n")
                ProcessBuilder("logcat", "-d").start()
                    .inputStream.reader().use { it.copyTo(file) }
            }
            showSnackbar(logFile.toString())
        }
    }

    fun clearMagiskLog() = repo.clearMagiskLogs {
        showSnackbar(R.string.logs_cleared)
        startLoading()
    }

    fun clearLog() = viewModelScope.launch {
        repo.clearLogs()
        showSnackbar(R.string.logs_cleared)
        startLoading()
    }
}
