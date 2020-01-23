package com.topjohnwu.magisk.utils

import androidx.databinding.Observable
import androidx.databinding.ObservableField
import com.topjohnwu.magisk.model.observer.Observer
import java.io.Serializable

/**
 * Kotlin version of [ObservableField].
 * You can define if wrapped type is Nullable or not.
 * You can use kotlin get/set syntax for value
 *
 * ## Notes
 * This stays final for fuck's sake. Too many things depend on it, so you just cannot go around and
 * change it randomly. Even though you think you're improving the design, you might be fucking this
 * up in unimaginable ways. So DON'T TOUCH THIS.
 *
 * In order to have value-less observer you need - you guessed it - **a fucking [Observer]**!
 */
class KObservableField<T> : ObservableField<T>, Serializable {

    var value: T
        set(value) {
            if (field != value) {
                field = value
                notifyChange()
            }
        }

    constructor(init: T) {
        value = init
    }

    constructor(init: T, vararg dependencies: Observable) : super(*dependencies) {
        value = init
    }

    @Deprecated(
        message = "Needed for data binding, use KObservableField.value syntax from code",
        replaceWith = ReplaceWith("value")
    )
    override fun get(): T {
        return value
    }

    @Deprecated(
        message = "Needed for data binding, use KObservableField.value = ... syntax from code",
        replaceWith = ReplaceWith("value = newValue")
    )
    override fun set(newValue: T) {
        value = newValue
    }

    override fun toString(): String {
        return "KObservableField(value=$value)"
    }
}
