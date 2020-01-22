package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.core.magiskdb.SettingsDao
import com.topjohnwu.magisk.core.magiskdb.StringDao
import io.reactivex.schedulers.Schedulers
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

interface DBConfig {
    val settingsDao: SettingsDao
    val stringDao: StringDao

    fun dbSettings(
        name: String,
        default: Int
    ) = DBSettingsValue(name, default)

    fun dbSettings(
        name: String,
        default: Boolean
    ) = DBBoolSettings(name, default)

    fun dbStrings(
        name: String,
        default: String,
        sync: Boolean = false
    ) = DBStringsValue(name, default, sync)

}

class DBSettingsValue(
    private val name: String,
    private val default: Int
) : ReadWriteProperty<DBConfig, Int> {

    private var value: Int? = null

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Int {
        if (value == null)
            value = thisRef.settingsDao.fetch(name, default).blockingGet()
        return value!!
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Int) {
        synchronized(this) {
            this.value = value
        }
        thisRef.settingsDao.put(name, value)
            .subscribeOn(Schedulers.io())
            .subscribe()
    }
}

class DBBoolSettings(
    name: String,
    default: Boolean
) : ReadWriteProperty<DBConfig, Boolean> {

    val base = DBSettingsValue(name, if (default) 1 else 0)

    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Boolean =
        base.getValue(thisRef, property) != 0

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Boolean) =
        base.setValue(thisRef, property, if (value) 1 else 0)
}

class DBStringsValue(
    private val name: String,
    private val default: String,
    private val sync: Boolean
) : ReadWriteProperty<DBConfig, String> {

    private var value: String? = null

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): String {
        if (value == null)
            value = thisRef.stringDao.fetch(name, default).blockingGet()
        return value!!
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: String) {
        synchronized(this) {
            this.value = value
        }
        if (value.isEmpty()) {
            if (sync) {
                thisRef.stringDao.delete(name).blockingAwait()
            } else {
                thisRef.stringDao.delete(name)
                    .subscribeOn(Schedulers.io())
                    .subscribe()
            }
        } else {
            if (sync) {
                thisRef.stringDao.put(name, value).blockingAwait()
            } else {
                thisRef.stringDao.put(name, value)
                    .subscribeOn(Schedulers.io())
                    .subscribe()
            }
        }
    }
}
