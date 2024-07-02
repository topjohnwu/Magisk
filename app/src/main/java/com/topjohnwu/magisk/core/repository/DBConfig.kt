package com.topjohnwu.magisk.core.repository

import com.topjohnwu.magisk.core.data.magiskdb.SettingsDao
import com.topjohnwu.magisk.core.data.magiskdb.StringDao
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

interface DBConfig {
    val settingsDB: SettingsDao
    val stringDB: StringDao
    val coroutineScope: CoroutineScope

    fun dbSettings(
        name: String,
        default: Int
    ) = IntDBProperty(name, default)

    fun dbSettings(
        name: String,
        default: Boolean
    ) = BoolDBProperty(name, default)

    fun dbStrings(
        name: String,
        default: String,
        sync: Boolean = false
    ) = StringDBProperty(name, default, sync)

}

class IntDBProperty(
    private val name: String,
    private val default: Int
) : ReadWriteProperty<DBConfig, Int> {

    var value: Int? = null

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Int {
        if (value == null)
            value = runBlocking { thisRef.settingsDB.fetch(name, default) }
        return value as Int
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Int) {
        synchronized(this) {
            this.value = value
        }
        thisRef.coroutineScope.launch {
            thisRef.settingsDB.put(name, value)
        }
    }
}

open class BoolDBProperty(
    name: String,
    default: Boolean
) : ReadWriteProperty<DBConfig, Boolean> {

    val base = IntDBProperty(name, if (default) 1 else 0)

    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Boolean =
        base.getValue(thisRef, property) != 0

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Boolean) =
        base.setValue(thisRef, property, if (value) 1 else 0)
}

class StringDBProperty(
    private val name: String,
    private val default: String,
    private val sync: Boolean
) : ReadWriteProperty<DBConfig, String> {

    private var value: String? = null

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): String {
        if (value == null)
            value = runBlocking {
                thisRef.stringDB.fetch(name, default)
            }
        return value!!
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: String) {
        synchronized(this) {
            this.value = value
        }
        if (value.isEmpty()) {
            if (sync) {
                runBlocking {
                    thisRef.stringDB.delete(name)
                }
            } else {
                thisRef.coroutineScope.launch {
                    thisRef.stringDB.delete(name)
                }
            }
        } else {
            if (sync) {
                runBlocking {
                    thisRef.stringDB.put(name, value)
                }
            } else {
                thisRef.coroutineScope.launch {
                    thisRef.stringDB.put(name, value)
                }
            }
        }
    }
}
