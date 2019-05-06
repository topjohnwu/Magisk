package com.topjohnwu.magisk.data.database

import com.topjohnwu.magisk.data.database.base.*

class SettingsDao : BaseDao() {

    override val table = DatabaseDefinition.Table.SETTINGS

    fun delete(key: String) = query<Delete> {
        condition { equals("key", key) }
    }.ignoreElement()

    fun put(key: String, value: Int) = query<Insert> {
        values(key to value.toString())
    }.ignoreElement()

    fun fetch(key: String) = query<Select> {
        condition { equals("key", key) }
    }.map { it.first().values.first().toIntOrNull() ?: -1 }

}