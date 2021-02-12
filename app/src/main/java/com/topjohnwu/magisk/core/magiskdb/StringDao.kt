package com.topjohnwu.magisk.core.magiskdb

class StringDao : BaseDao() {

    override val table = Table.STRINGS

    suspend fun delete(key: String) = buildQuery<Delete> {
        condition { equals("key", key) }
    }.commit()

    suspend fun put(key: String, value: String) = buildQuery<Replace> {
        values("key" to key, "value" to value)
    }.commit()

    suspend fun fetch(key: String, default: String = "") = buildQuery<Select> {
        fields("value")
        condition { equals("key", key) }
    }.query {
        it["value"]
    }.firstOrNull() ?: default

}
