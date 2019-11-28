package com.topjohnwu.magisk.model.preference

import androidx.core.content.edit
import com.topjohnwu.magisk.extensions.trimEmptyToNull
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

class StringProperty(
    private val name: String,
    private val default: String,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, String> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): String {
        val prefName = name.trimEmptyToNull() ?: property.name
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: String
    ) {
        val prefName = name.trimEmptyToNull() ?: property.name
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}