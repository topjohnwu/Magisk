package com.topjohnwu.magisk.databinding

import androidx.databinding.Observable
import androidx.databinding.PropertyChangeRegistry

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

fun ObservableHost.addOnPropertyChangedCallback(
    fieldId: Int,
    removeAfterChanged: Boolean = false,
    callback: () -> Unit
) {
    addOnPropertyChangedCallback(object : Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
            if (fieldId == propertyId) {
                callback()
                if (removeAfterChanged)
                    removeOnPropertyChangedCallback(this)
            }
        }
    })
}

/**
 * Injects boilerplate implementation for {@literal @}[androidx.databinding.Bindable] field setters.
 *
 * # Examples:
 * ```kotlin
 * @get:Bindable
 * var myField = defaultValue
 *     set(value) = set(value, field, { field = it }, BR.myField) {
 *         doSomething(it)
 *     }
 * ```
 * */

inline fun <reified T> ObservableHost.set(
    new: T, old: T, setter: (T) -> Unit, fieldId: Int, afterChanged: (T) -> Unit = {}) {
    if (old != new) {
        setter(new)
        notifyPropertyChanged(fieldId)
        afterChanged(new)
    }
}

inline fun <reified T> ObservableHost.set(
    new: T, old: T, setter: (T) -> Unit, vararg fieldIds: Int, afterChanged: (T) -> Unit = {}) {
    if (old != new) {
        setter(new)
        fieldIds.forEach { notifyPropertyChanged(it) }
        afterChanged(new)
    }
}
