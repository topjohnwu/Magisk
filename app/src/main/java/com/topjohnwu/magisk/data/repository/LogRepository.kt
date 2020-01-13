package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.database.SuLogDao
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import io.reactivex.Single


class LogRepository(
    private val logDao: SuLogDao
) {

    fun fetchLogs() = logDao.fetchAll()

    fun fetchMagiskLogs() = Single.fromCallable {
        Shell.su("tail -n 5000 ${Const.MAGISK_LOG}").exec().out
    }.flattenAsFlowable { it }.filter { it.isNotEmpty() }

    fun clearLogs() = logDao.deleteAll()

    fun clearMagiskLogs() = Completable.fromAction {
        Shell.su("echo -n > ${Const.MAGISK_LOG}").exec()
    }

    fun insert(log: MagiskLog) = logDao.insert(log)

}
