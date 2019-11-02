package com.topjohnwu.magisk.extensions

import androidx.databinding.Observable
import androidx.databinding.ObservableBoolean
import androidx.databinding.ObservableField
import androidx.databinding.ObservableInt

fun <T> ObservableField<T>.addOnPropertyChangedCallback(
    removeAfterChanged: Boolean = false,
    callback: (T?) -> Unit
) {
    addOnPropertyChangedCallback(object : Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
            callback(get())
            if (removeAfterChanged) removeOnPropertyChangedCallback(this)
        }
    })
}

fun ObservableInt.addOnPropertyChangedCallback(
    removeAfterChanged: Boolean = false,
    callback: (Int) -> Unit
) {
    addOnPropertyChangedCallback(object : Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
            callback(get())
            if (removeAfterChanged) removeOnPropertyChangedCallback(this)
        }
    })
}

fun ObservableBoolean.addOnPropertyChangedCallback(
    removeAfterChanged: Boolean = false,
    callback: (Boolean) -> Unit
) {
    addOnPropertyChangedCallback(object : Observable.OnPropertyChangedCallback() {
        override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
            callback(get())
            if (removeAfterChanged) removeOnPropertyChangedCallback(this)
        }
    })
}

inline fun <T> ObservableField<T>.update(block: (T?) -> Unit) {
    set(get().apply(block))
}

inline fun <T> ObservableField<T>.updateNonNull(block: (T) -> Unit) {
    update {
        it ?: return@update
        block(it)
    }
}

inline fun ObservableInt.update(block: (Int) -> Unit) {
    set(get().apply(block))
}