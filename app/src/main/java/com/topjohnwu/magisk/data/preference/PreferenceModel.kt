package com.topjohnwu.magisk.data.preference

import android.content.Context
import android.content.SharedPreferences
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

interface PreferenceModel {

    val context: Context

    val fileName: String
        get() = "${context.packageName}_preferences"

    val prefs: SharedPreferences
        get() = context.getSharedPreferences(fileName, Context.MODE_PRIVATE)

    fun preferenceStrInt(
        name: String,
        default: Int,
        commit: Boolean = false
    ) = object: ReadWriteProperty<PreferenceModel, Int> {
        val base = StringProperty(name, default.toString(), commit)
        override fun getValue(thisRef: PreferenceModel, property: KProperty<*>): Int =
                base.getValue(thisRef, property).toInt()

        override fun setValue(thisRef: PreferenceModel, property: KProperty<*>, value: Int) =
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
