package com.topjohnwu.magisk.data.database

import com.topjohnwu.magisk.data.database.base.*
import com.topjohnwu.magisk.model.entity.MagiskLog
import com.topjohnwu.magisk.model.entity.toLog
import com.topjohnwu.magisk.model.entity.toMap
import java.util.concurrent.TimeUnit

class LogDao : BaseDao() {

    override val table = DatabaseDefinition.Table.LOG

    fun deleteOutdated(
        suTimeout: Long = TimeUnit.DAYS.toMillis(14)
    ) = query<Delete> {
        condition {
            lessThan("time", suTimeout.toString())
        }
    }.ignoreElement()

    fun deleteAll() = query<Delete> {}.ignoreElement()

    fun fetchAll() = query<Select> {
        orderBy("time", Order.DESC)
    }.flattenAsFlowable { it }
        .map { it.toLog() }
        .toList()

    fun put(log: MagiskLog) = query<Insert> {
        values(log.toMap())
    }.ignoreElement()

}