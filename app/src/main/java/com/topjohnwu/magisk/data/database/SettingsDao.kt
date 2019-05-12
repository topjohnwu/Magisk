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

    fun fetch(key: String, default: Int = -1) = query<Select> {
        condition { equals("key", key) }
    }.map { it.firstOrNull()?.values?.firstOrNull()?.toIntOrNull() ?: default }

}