package com.topjohnwu.magisk.model.preference

import androidx.core.content.edit
import com.topjohnwu.magisk.extensions.trimEmptyToNull
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

class FloatProperty(
    private val name: String,
    private val default: Float,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Float> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Float {
        val prefName = name.trimEmptyToNull() ?: property.name
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Float
    ) {
        val prefName = name.trimEmptyToNull() ?: property.name
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}