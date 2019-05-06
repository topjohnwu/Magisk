package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.Constants
import com.topjohnwu.magisk.data.database.LogDao
import com.topjohnwu.magisk.data.database.base.suRaw
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.magisk.model.entity.WrappedMagiskLog
import timber.log.Timber
import java.util.concurrent.TimeUnit


class LogRepository(
    private val logDao: LogDao
) {

    fun fetchLogs() = logDao.fetchAll()
        .map { it.sortByDescending { it.date.time }; it }
        .map { it.wrap() }

    fun fetchMagiskLogs() = "tail -n 5000 ${Constants.MAGISK_LOG}".suRaw()
        .filter { it.isNotEmpty() }
        .map { Timber.i(it.toString()); it }

    private fun List<MagiskLog>.wrap(): List<WrappedMagiskLog> {
        val day = TimeUnit.DAYS.toMillis(1)
        var currentDay = firstOrNull()?.date?.time ?: return listOf()
        var tempList = this
        val outList = mutableListOf<WrappedMagiskLog>()

        while (tempList.isNotEmpty()) {
            val logsGivenDay = takeWhile { it.date.time / day == currentDay / day }
            currentDay = tempList.firstOrNull()?.date?.time ?: currentDay + day

            if (logsGivenDay.isEmpty())
                continue

            outList.add(WrappedMagiskLog(currentDay / day * day, logsGivenDay))
            tempList = tempList.subList(logsGivenDay.size, tempList.size)
        }

        return outList
    }


}