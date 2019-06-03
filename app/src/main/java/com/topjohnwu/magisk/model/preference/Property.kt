package com.topjohnwu.magisk.model.preference

import android.content.SharedPreferences

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