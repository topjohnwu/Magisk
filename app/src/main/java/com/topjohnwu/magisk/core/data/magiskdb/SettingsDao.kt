package com.topjohnwu.magisk.core.data.magiskdb

class SettingsDao : MagiskDB() {

    suspend fun delete(key: String) {
        val query = "DELETE FROM ${Table.SETTINGS} WHERE key == \"$key\""
        exec(query)
    }

    suspend fun put(key: String, value: Int) {
        val kv = mapOf("key" to key, "value" to value)
        val query = "REPLACE INTO ${Table.SETTINGS} ${kv.toQuery()}"
        exec(query)
    }

    suspend fun fetch(key: String, default: Int = -1): Int {
        val query = "SELECT value FROM ${Table.SETTINGS} WHERE key == \"$key\" LIMIT 1"
        return exec(query) { it["value"]?.toInt() }.firstOrNull() ?: default
    }
}
