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
        default: Int,
        commit: Boolean = false
    ) = IntProperty(name, default, commit)

    fun preference(
        name: String,
        default: String,
        commit: Boolean = false
    ) = StringProperty(name, default, commit)
}

class BooleanProperty(
    private val name: String,
    private val default: Boolean,
    private val commit: Boolean
) : ReadWriteProperty<PreferenceConfig, Boolean> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Boolean {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.getBoolean(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>,
        value: Boolean
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { putBoolean(prefName, value) }
    }
}

class IntProperty(
    private val name: String,
    private val default: Int,
    private val commit: Boolean
) : ReadWriteProperty<PreferenceConfig, Int> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): Int {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.getInt(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>,
        value: Int
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { putInt(prefName, value) }
    }
}

class StringProperty(
    private val name: String,
    private val default: String,
    private val commit: Boolean
) : ReadWriteProperty<PreferenceConfig, String> {

    override operator fun getValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>
    ): String {
        val prefName = name.ifBlank { property.name }
        return thisRef.prefs.getString(prefName, default) ?: default
    }

    override operator fun setValue(
        thisRef: PreferenceConfig,
        property: KProperty<*>,
        value: String
    ) {
        val prefName = name.ifBlank { property.name }
        thisRef.prefs.edit(commit) { putString(prefName, value) }
    }
}
