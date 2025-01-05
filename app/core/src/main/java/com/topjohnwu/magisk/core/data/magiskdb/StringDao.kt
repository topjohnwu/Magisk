package com.topjohnwu.magisk.core.data.magiskdb

class StringDao : MagiskDB() {

    suspend fun delete(key: String) {
        val query = "DELETE FROM ${Table.STRINGS} WHERE key=\"$key\""
        exec(query)
    }

    suspend fun put(key: String, value: String) {
        val kv = mapOf("key" to key, "value" to value)
        val query = "REPLACE INTO ${Table.STRINGS} ${kv.toQuery()}"
        exec(query)
    }

    suspend fun fetch(key: String, default: String = ""): String {
        val query = "SELECT value FROM ${Table.STRINGS} WHERE key=\"$key\" LIMIT 1"
        return exec(query) { it["value"] }.firstOrNull() ?: default
    }
}
