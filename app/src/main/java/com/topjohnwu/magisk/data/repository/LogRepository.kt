package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.data.database.SuLogDao
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.magisk.model.entity.WrappedMagiskLog
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import io.reactivex.Single
import java.util.concurrent.TimeUnit


class LogRepository(
    private val logDao: SuLogDao
) {

    fun fetchLogs() = logDao.fetchAll().map { it.wrap() }

    fun fetchMagiskLogs() = Single.fromCallable {
        Shell.su("tail -n 5000 ${Const.MAGISK_LOG}").exec().out
    }.flattenAsFlowable { it }.filter { it.isNotEmpty() }

    fun clearLogs() = logDao.deleteAll()

    fun clearMagiskLogs() = Completable.fromAction {
        Shell.su("echo -n > ${Const.MAGISK_LOG}").exec()
    }

    fun insert(log: MagiskLog) = logDao.insert(log)

    private fun List<MagiskLog>.wrap(): List<WrappedMagiskLog> {
        val day = TimeUnit.DAYS.toMillis(1)
        return groupBy { it.time / day }
            .map { WrappedMagiskLog(it.key * day, it.value) }
    }


}
