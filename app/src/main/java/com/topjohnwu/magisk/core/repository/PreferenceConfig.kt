package com.topjohnwu.magisk.core.repository

import android.content.Context
import android.content.SharedPreferences
import androidx.core.content.edit
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

interface PreferenceConfig {

    val context: Context

    val fileName: String
        get() = "${context.packageName}_preferences"

    val prefs: SharedPreferences
        get() = context.getSharedPreferences(fileName, Context.MODE_PRIVATE)

    fun preferenceStrInt(
        name: String,
        default: Int,
        commit: Boolean = false
    ) = object: ReadWriteProperty<PreferenceConfig, Int> {
        val base = StringProperty(name, default.toString(), commit)
        override fun getValue(thisRef: PreferenceConfig, property: KProperty<*>): Int =
                base.getValue(thisRef, property).toInt()

        override fun setValue(thisRef: PreferenceConfig, property: KProperty<*>, value: Int) =
                base.setValue(thisRef, property, value.toString())
    }

    fun preference(
        name: String,
        default: Boolean,
        commit: Boolean = false
    ) = BooleanProperty(name, default, commit)

    fun preference(
        name: String,
        default: Float,
        commit: Boolean = false
    ) = FloatProperty(name, default, commit)

    fun preference(
        name: String,
        default: Int,
        commit: Boolean = false
    ) = IntProperty(name, default, commit)

    fun preference(
        name: String,
        default: Long,
        commit: Boolean = false
    ) = LongProperty(name, default, commit)

    fun preference(
        name: String,
        default: String,
        commit: Boolean = false
    ) = StringProperty(name, default, commit)

    fun preference(
        name: String,
        default: Set<String>,
        commit: Boolean = false
    ) = StringSetProperty(name, default, commit)

}

abstract class PreferenceProperty {

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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, Boolean> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Boolean {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, Float> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Float {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, Int> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Int {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, Long> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Long {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, String> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): String {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
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
) : PreferenceProperty(), ReadWriteProperty<PreferenceConfig, Set<String>> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Set<String> {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>,
        value: Set<String>
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}
