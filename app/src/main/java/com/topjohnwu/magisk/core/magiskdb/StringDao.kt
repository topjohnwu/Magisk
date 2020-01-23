package com.topjohnwu.magisk.core.magiskdb

class StringDao : BaseDao() {

    override val table = Table.STRINGS

    fun delete(key: String) = query<Delete> {
        condition { equals("key", key) }
    }.ignoreElement()

    fun put(key: String, value: String) = query<Replace> {
        values("key" to key, "value" to value)
    }.ignoreElement()

    fun fetch(key: String, default: String = "") = query<Select> {
        fields("value")
        condition { equals("key", key) }
    }.map { it.firstOrNull()?.values?.firstOrNull() ?: default }

}
