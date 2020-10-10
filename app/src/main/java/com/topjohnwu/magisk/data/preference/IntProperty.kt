package com.topjohnwu.magisk.data.preference

import androidx.core.content.edit
import com.topjohnwu.magisk.ktx.trimEmptyToNull
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

class IntProperty(
    private val name: String,
    private val default: Int,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Int> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Int {
        val prefName = name.trimEmptyToNull() ?: property.name
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Int
    ) {
        val prefName = name.trimEmptyToNull() ?: property.name
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}
