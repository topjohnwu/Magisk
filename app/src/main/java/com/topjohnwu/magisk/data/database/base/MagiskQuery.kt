package com.topjohnwu.magisk.data.database.base

inline class MagiskQuery(private val _query: String) {
    val query get() = "magisk --sqlite '$_query'"
}