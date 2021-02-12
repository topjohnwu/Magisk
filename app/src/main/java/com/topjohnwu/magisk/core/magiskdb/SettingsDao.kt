package com.topjohnwu.magisk.core.magiskdb

class SettingsDao : BaseDao() {

    override val table = Table.SETTINGS

    suspend fun delete(key: String) = buildQuery<Delete> {
        condition { equals("key", key) }
    }.commit()

    suspend fun put(key: String, value: Int) = buildQuery<Replace> {
        values("key" to key, "value" to value)
    }.commit()

    suspend fun fetch(key: String, default: Int = -1) = buildQuery<Select> {
        fields("value")
        condition { equals("key", key) }
    }.query {
        it["value"]?.toIntOrNull()
    }.firstOrNull() ?: default

}
