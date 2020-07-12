package com.topjohnwu.magisk.utils

import androidx.databinding.Observable
import androidx.databinding.PropertyChangeRegistry
import kotlin.properties.ReadWriteProperty
import kotlin.reflect.KProperty

/**
 * Modified from https://github.com/skoumalcz/teanity/blob/1.2/core/src/main/java/com/skoumal/teanity/observable/Notifyable.kt
 *
 * Interface that allows user to be observed via DataBinding or manually by assigning listeners.
 *
 * @see [androidx.databinding.Observable]
 * */
interface ObservableHost : Observable {

    var callbacks: PropertyChangeRegistry?

    /**
     * Notifies all observers that something has changed. By default implementation this method is
     * synchronous, hence observers will never be notified in undefined order. Observers might
     * choose to refresh the view completely, which is beyond the scope of this function.
     * */
    fun notifyChange() {
        synchronized(this) {
            callbacks ?: return
        }.notifyCallbacks(this, 0, null)
    }

    /**
     * Notifies all observers about field with [fieldId] has been changed. This will happen
     * synchronously before or after [notifyChange] has been called. It will never be called during
     * the execution of aforementioned method.
     * */
    fun notifyPropertyChanged(fieldId: Int) {
        synchronized(this) {
            callbacks ?: return
        }.notifyCallbacks(this, fieldId, null)
    }

    override fun addOnPropertyChangedCallback(callback: Observable.OnPropertyChangedCallback) {
        synchronized(this) {
            callbacks ?: PropertyChangeRegistry().also { callbacks = it }
        }.add(callback)
    }

    override fun removeOnPropertyChangedCallback(callback: Observable.OnPropertyChangedCallback) {
        synchronized(this) {
            callbacks ?: return
        }.remove(callback)
    }
}

/**
 * Declares delegated property in [ObservableHost] parent. This property is available for DataBinding
 * to be observed as usual. The only caveat is that in order for binding to generate the [fieldId]
 * it has to be annotated accordingly.
 *
 * The annotation however give very strict control over your internal fields and overall reduce
 * overhead in notifying observers. (In comparison to [androidx.databinding.ObservableField])
 * It helps the kotlin code to feel more,... _native_, while respecting the original functionality.
 *
 * # Examples:
 *
 * ## The most basic usage would probably be:
 * ```kotlin
 * @get:Bindable
 * var myField by observable(defaultValue, BR.myField)
 *      private set
 * ```
 *
 * ## You can use the field as public read/write, of course:
 * ```kotlin
 * @get:Bindable
 * var myField by observable(defaultValue, BR.myField)
 * ```
 *
 * ## Please beware that delegated property instantiates one class per property
 * We discourage using simple getters via delegated properties. Instead you can do something like
 * this:
 *
 * ```kotlin
 * @get:Bindable
 * var myField by observable(defaultValue, BR.myField, BR.myTransformedField)
 *
 * var myTransformedField
 *      @Bindable get() {
 *          return myField.transform()
 *      }
 *      set(value) {
 *          myField = value.transform()
 *      }
 * ```
 *
 * */

// Optimize for the most common use case
// Generic type is reified to optimize primitive types
inline fun <reified T> ObservableHost.observable(
    initialValue: T,
    fieldId: Int
) = object : ReadWriteProperty<ObservableHost, T> {
    private var field = initialValue

    override fun getValue(thisRef: ObservableHost, property: KProperty<*>): T {
        return field
    }

    @Synchronized
    override fun setValue(thisRef: ObservableHost, property: KProperty<*>, value: T) {
        if (field != value) {
            field = value
            notifyPropertyChanged(fieldId)
        }
    }
}

inline fun <reified T> ObservableHost.observable(
    initialValue: T,
    vararg fieldIds: Int
) = object : ReadWriteProperty<ObservableHost, T> {
    private var field = initialValue

    override fun getValue(thisRef: ObservableHost, property: KProperty<*>): T {
        return field
    }

    @Synchronized
    override fun setValue(thisRef: ObservableHost, property: KProperty<*>, value: T) {
        if (field != value) {
            field = value
            fieldIds.forEach { notifyPropertyChanged(it) }
        }
    }
}

inline fun <reified T> ObservableHost.observable(
    initialValue: T,
    vararg fieldIds: Int,
    crossinline afterChanged: (T) -> Unit
) = object : ReadWriteProperty<ObservableHost, T> {
    private var field = initialValue

    override fun getValue(thisRef: ObservableHost, property: KProperty<*>): T {
        return field
    }

    @Synchronized
    override fun setValue(thisRef: ObservableHost, property: KProperty<*>, value: T) {
        if (field != value) {
            field = value
            fieldIds.forEach { notifyPropertyChanged(it) }
            afterChanged(value)
        }
    }
}
