package com.topjohnwu.magisk.data.preference

import androidx.core.content.edit
import com.topjohnwu.magisk.ktx.trimEmptyToNull
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

class LongProperty(
    private val name: String,
    private val default: Long,
    private val commit: Boolean
) : Property(), ReadWriteProperty<PreferenceModel, Long> {

    override operator fun getValue(
        thisRef: PreferenceModel,
        property: KProperty<*>
    ): Long {
        val prefName = name.trimEmptyToNull() ?: property.name
        return thisRef.prefs.get(prefName, default)
    }

    override operator fun setValue(
        thisRef: PreferenceModel,
        property: KProperty<*>,
        value: Long
    ) {
        val prefName = name.trimEmptyToNull() ?: property.name
        thisRef.prefs.edit(commit) { put(prefName, value) }
    }
}
