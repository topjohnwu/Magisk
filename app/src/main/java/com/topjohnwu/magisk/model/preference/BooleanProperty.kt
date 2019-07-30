package com.topjohnwu.magisk.model.preference

import androidx.core.content.edit
import com.topjohnwu.magisk.extensions.trimEmptyToNull
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

class BooleanProperty(
    private val name: String,
    private val default: Boolean,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Boolean> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Boolean {
        val prefName = name.trimEmptyToNull() ?: property.name
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Boolean
    ) {
        val prefName = name.trimEmptyToNull() ?: property.name
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}