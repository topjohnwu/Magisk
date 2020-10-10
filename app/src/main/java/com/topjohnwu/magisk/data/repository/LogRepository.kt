package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.model.su.SuLog
import com.topjohnwu.magisk.data.database.SuLogDao
import com.topjohnwu.magisk.ktx.await
import com.topjohnwu.superuser.Shell


class LogRepository(
    private val logDao: SuLogDao
) {

    suspend fun fetchSuLogs() = logDao.fetchAll()

    suspend fun fetchMagiskLogs(): String {
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
        Shell.su("cat ${Const.MAGISK_LOG}").to(list).await()
        return list.buf.toString()
    }

    suspend fun clearLogs() = logDao.deleteAll()

    fun clearMagiskLogs(cb: (Shell.Result) -> Unit) =
        Shell.su("echo -n > ${Const.MAGISK_LOG}").submit(cb)

    suspend fun insert(log: SuLog) = logDao.insert(log)

}
