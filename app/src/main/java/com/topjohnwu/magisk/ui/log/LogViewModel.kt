package com.topjohnwu.magisk.ui.log

import androidx.databinding.ObservableField
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.ktx.value
import com.topjohnwu.magisk.model.entity.recycler.LogItem
import com.topjohnwu.magisk.model.entity.recycler.TextItem
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.*
import timber.log.Timber
import java.io.File
import java.io.IOException
import java.util.*

class LogViewModel(
    private val repo: LogRepository
) : BaseViewModel() {

    // --- empty view

    val itemEmpty = TextItem(R.string.log_data_none)
    val itemMagiskEmpty = TextItem(R.string.log_data_magisk_none)

    // --- su log

    val items = diffListOf<LogItem>()
    val itemBinding = itemBindingOf<LogItem> {
        it.bindExtra(BR.viewModel, this)
    }

    // --- magisk log

    val consoleText = ObservableField(" ")

    override fun refresh() = viewModelScope.launch {
        consoleText.value = repo.fetchMagiskLogs()
        val deferred = withContext(Dispatchers.Default) {
            async {
                val suLogs = repo.fetchSuLogs().map { LogItem(it) }
                suLogs to items.calculateDiff(suLogs)
            }
        }
        delay(500)
        val (suLogs, diff) = deferred.await()
        items.firstOrNull()?.isTop = false
        items.lastOrNull()?.isBottom = false
        items.update(suLogs, diff)
        items.firstOrNull()?.isTop = true
        items.lastOrNull()?.isBottom = true
    }

    fun saveMagiskLog() {
        val now = Calendar.getInstance()
        val filename = "magisk_log_%04d%02d%02d_%02d%02d%02d.log".format(
            now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
            now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
            now.get(Calendar.MINUTE), now.get(Calendar.SECOND)
        )

        val logFile = File(Config.downloadDirectory, filename)
        try {
            logFile.createNewFile()
        } catch (e: IOException) {
            Timber.e(e)
            return
        }

        Shell.su("cat ${Const.MAGISK_LOG} > $logFile").submit {
            SnackbarEvent(logFile.path).publish()
        }
    }

    fun clearMagiskLog() = repo.clearMagiskLogs {
        SnackbarEvent(R.string.logs_cleared).publish()
        requestRefresh()
    }

    fun clearLog() = viewModelScope.launch {
        repo.clearLogs()
        SnackbarEvent(R.string.logs_cleared).publish()
        requestRefresh()
    }
}
