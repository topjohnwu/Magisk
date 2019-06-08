package com.topjohnwu.magisk.model.preference

import android.content.Context
import android.content.SharedPreferences

interface PreferenceModel {

    val context: Context

    val fileName: String
        get() = "${context.packageName}_preferences"
    val commitPrefs: Boolean
        get() = false
    val prefs: SharedPreferences
        get() = context.getSharedPreferences(fileName, Context.MODE_PRIVATE)

    fun preference(
        name: String,
        default: Boolean,
        commit: Boolean = commitPrefs
    ) = BooleanProperty(name, default, commit)

    fun preference(
        name: String,
        default: Float,
        commit: Boolean = commitPrefs
    ) = FloatProperty(name, default, commit)

    fun preference(
        name: String,
        default: Int,
        commit: Boolean = commitPrefs
    ) = IntProperty(name, default, commit)

    fun preference(
        name: String,
        default: Long,
        commit: Boolean = commitPrefs
    ) = LongProperty(name, default, commit)

    fun preference(
        name: String,
        default: String,
        commit: Boolean = commitPrefs
    ) = StringProperty(name, default, commit)

    fun preference(
        name: String,
        default: Set<String>,
        commit: Boolean = commitPrefs
    ) = StringSetProperty(name, default, commit)

}
