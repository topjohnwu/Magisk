package com.topjohnwu.magisk.data.database.base

import androidx.annotation.AnyThread
import com.topjohnwu.superuser.Shell
import io.reactivex.Single

object DatabaseDefinition {

    object Table {
        const val POLICY = "policies"
        const val LOG = "logs"
        const val SETTINGS = "settings"
        const val STRINGS = "strings"
    }

}

@AnyThread
fun MagiskQuery.query() = query.su()

fun String.suRaw() = Single.fromCallable { Shell.su(this).exec().out }
fun String.su() = suRaw().map { it.toMap() }

fun List<String>.toMap() = map { it.split(Regex("\\|")) }
    .map { it.toMapInternal() }

private fun List<String>.toMapInternal() = map { it.split("=", limit = 2) }
    .filter { it.size == 2 }
    .map { Pair(it[0], it[1]) }
    .toMap()