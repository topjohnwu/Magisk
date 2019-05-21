package com.topjohnwu.magisk.data.database.base

abstract class BaseDao {

    abstract val table: String

    inline fun <reified Builder : MagiskQueryBuilder> query(builder: Builder.() -> Unit) =
        Builder::class.java.newInstance()
            .apply { table = this@BaseDao.table }
            .apply(builder)
            .toString()
            .let { MagiskQuery(it) }
            .query()

}