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
        val list = object : AbstractMutableList<String>() {
            val buf = StringBuilder()
            override val size get() = 0
            override fun get(index: Int): String = ""
            override fun removeAt(index: Int): String = ""
            override fun set(index: Int, element: String): String = ""
            override fun add(index: Int, element: String) {
                if (element.isNotEmpty()) {
                    buf.append(element)
                    buf.append('\n')
                }
            }
        }
        Shell.su("cat ${Const.MAGISK_LOG}").to(list).exec()
        list.buf.toString()
    }

    fun clearLogs() = logDao.deleteAll()

    fun clearMagiskLogs() = Completable.fromAction {
        Shell.su("echo -n > ${Const.MAGISK_LOG}").exec()
    }

    fun insert(log: MagiskLog) = logDao.insert(log)

}
