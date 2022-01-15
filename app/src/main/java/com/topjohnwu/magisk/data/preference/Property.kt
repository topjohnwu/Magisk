package com.topjohnwu.magisk.data.preference

import android.content.SharedPreferences
import androidx.core.content.edit
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

abstract class Property {

    fun SharedPreferences.Editor.put(name: String, value: Boolean) = putBoolean(name, value)
    fun SharedPreferences.Editor.put(name: String, value: Float) = putFloat(name, value)
    fun SharedPreferences.Editor.put(name: String, value: Int) = putInt(name, value)
    fun SharedPreferences.Editor.put(name: String, value: Long) = putLong(name, value)
    fun SharedPreferences.Editor.put(name: String, value: String) = putString(name, value)
    fun SharedPreferences.Editor.put(name: String, value: Set<String>) = putStringSet(name, value)

    fun SharedPreferences.get(name: String, value: Boolean) = getBoolean(name, value)
    fun SharedPreferences.get(name: String, value: Float) = getFloat(name, value)
    fun SharedPreferences.get(name: String, value: Int) = getInt(name, value)
    fun SharedPreferences.get(name: String, value: Long) = getLong(name, value)
    fun SharedPreferences.get(name: String, value: String) = getString(name, value) ?: value
    fun SharedPreferences.get(name: String, value: Set<String>) = getStringSet(name, value) ?: value

}

class BooleanProperty(
    private val name: String,
    private val default: Boolean,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Boolean> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Boolean {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Boolean
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}

class FloatProperty(
    private val name: String,
    private val default: Float,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Float> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Float {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Float
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}

class IntProperty(
    private val name: String,
    private val default: Int,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Int> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Int {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Int
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}

class LongProperty(
    private val name: String,
    private val default: Long,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Long> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Long {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Long
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}

class StringProperty(
    private val name: String,
    private val default: String,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, String> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): String {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: String
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}

class StringSetProperty(
    private val name: String,
    private val default: Set<String>,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Set<String>> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Set<String> {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Set<String>
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}
