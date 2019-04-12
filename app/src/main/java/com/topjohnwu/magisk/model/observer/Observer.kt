package com.topjohnwu.magisk.model.observer

import androidx.databinding.Observable
import androidx.databinding.ObservableField
import java.io.Serializable


class Observer<T>(vararg dependencies: Observable, private val observer: () -> T) :
    ObservableField<T>(*dependencies), Serializable {

    val value: T get() = observer()

    @Deprecated(
        message = "Use KObservableField.value syntax from code",
        replaceWith = ReplaceWith("value")
    )
    override fun get(): T {
        return value
    }

    @Deprecated(
        message = "Observer cannot be set",
        level = DeprecationLevel.HIDDEN
    )
    override fun set(newValue: T) {
    }

    override fun toString(): String {
        return "Observer(value=$value)"
    }
}