package com.topjohnwu.magisk.core.magiskdb

import androidx.annotation.StringDef

abstract class BaseDao {

    object Table {
        const val POLICY = "policies"
        const val LOG = "logs"
        const val SETTINGS = "settings"
        const val STRINGS = "strings"
    }

    @StringDef(Table.POLICY, Table.LOG, Table.SETTINGS, Table.STRINGS)
    @Retention(AnnotationRetention.SOURCE)
    annotation class TableStrict

    @TableStrict
    abstract val table: String

    inline fun <reified Builder : Query.Builder> buildQuery(builder: Builder.() -> Unit = {}) =
        Builder::class.java.newInstance()
            .apply { table = this@BaseDao.table }
            .apply(builder)
            .toString()
            .let { Query(it) }

}
