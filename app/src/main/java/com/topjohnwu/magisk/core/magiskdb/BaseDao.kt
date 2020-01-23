package com.topjohnwu.magisk.core.magiskdb

import androidx.annotation.StringDef
import com.topjohnwu.superuser.Shell
import io.reactivex.Single

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

    inline fun <reified Builder : Query.Builder> query(builder: Builder.() -> Unit = {}) =
        Builder::class.java.newInstance()
            .apply { table = this@BaseDao.table }
            .apply(builder)
            .toString()
            .let { Query(it) }
            .query()

}

fun Query.query() = query.su()

private fun String.suRaw() = Single.fromCallable { Shell.su(this).exec().out }
private fun String.su() = suRaw().map { it.toMap() }

private fun List<String>.toMap() = map { it.split(Regex("\\|")) }
    .map { it.toMapInternal() }

private fun List<String>.toMapInternal() = map { it.split("=", limit = 2) }
    .filter { it.size == 2 }
    .map { Pair(it[0], it[1]) }
    .toMap()
