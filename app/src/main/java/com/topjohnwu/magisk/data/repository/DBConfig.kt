package com.topjohnwu.magisk.data.repository

import com.topjohnwu.magisk.data.database.SettingsDao
import com.topjohnwu.magisk.data.database.StringDao
import com.topjohnwu.magisk.utils.trimEmptyToNull
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
        default: String
    ) = DBStringsValue(name, default)

}

class DBSettingsValue(
        private val name: String,
        private val default: Int
) : ReadWriteProperty<DBConfig, Int> {

    private var value: Int? = null

    private fun getKey(property: KProperty<*>) = name.trimEmptyToNull() ?: property.name

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Int {
        if (value == null)
            value = thisRef.settingsDao.fetch(getKey(property), default).blockingGet()
        return value!!
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Int) {
        synchronized(this) {
            this.value = value
        }
        thisRef.settingsDao.put(getKey(property), value)
                .subscribeOn(Schedulers.io())
                .subscribe()
    }
}

class DBBoolSettings(
        name: String,
        default: Boolean
) : ReadWriteProperty<DBConfig, Boolean> {

    val base = DBSettingsValue(name, if (default) 1 else 0)

    override fun getValue(thisRef: DBConfig, property: KProperty<*>): Boolean
            = base.getValue(thisRef, property) != 0

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: Boolean) =
            base.setValue(thisRef, property, if (value) 1 else 0)
}

class DBStringsValue(
        private val name: String,
        private val default: String
) : ReadWriteProperty<DBConfig, String> {

    private var value: String? = null

    private fun getKey(property: KProperty<*>) = name.trimEmptyToNull() ?: property.name

    @Synchronized
    override fun getValue(thisRef: DBConfig, property: KProperty<*>): String {
        if (value == null)
            value = thisRef.stringDao.fetch(getKey(property), default).blockingGet()
        return value!!
    }

    override fun setValue(thisRef: DBConfig, property: KProperty<*>, value: String) {
        synchronized(this) {
            this.value = value
        }
        thisRef.stringDao.put(getKey(property), value)
                .subscribeOn(Schedulers.io())
                .subscribe()
    }
}
