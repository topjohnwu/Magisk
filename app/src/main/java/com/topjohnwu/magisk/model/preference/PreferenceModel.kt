package com.topjohnwu.magisk.model.preference

import android.content.Context
import kotlin.properties.ReadWriteProperty

abstract class PreferenceModel(
    private val commitPrefs: Boolean = false
) {

    protected abstract val fileName: String
    protected abstract val context: Context

    internal val prefs get() = context.getSharedPreferences(fileName, Context.MODE_PRIVATE)

    protected fun preference(
        name: String,
        default: Boolean,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, Boolean> = BooleanProperty(name, default, commit)

    protected fun preference(
        name: String,
        default: Float,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, Float> = FloatProperty(name, default, commit)

    protected fun preference(
        name: String,
        default: Int,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, Int> = IntProperty(name, default, commit)

    protected fun preference(
        name: String,
        default: Long,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, Long> = LongProperty(name, default, commit)

    protected fun preference(
        name: String,
        default: String,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, String> = StringProperty(name, default, commit)

    protected fun preference(
        name: String,
        default: Set<String>,
        commit: Boolean = commitPrefs
    ): ReadWriteProperty<PreferenceModel, Set<String>> = StringSetProperty(name, default, commit)

}