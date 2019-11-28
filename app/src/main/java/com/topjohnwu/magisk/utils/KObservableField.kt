package com.topjohnwu.magisk.utils

import androidx.databinding.Observable
import androidx.databinding.ObservableField
import java.io.Serializable

/**
 * Kotlin version of [ObservableField].
 * You can define if wrapped type is Nullable or not.
 * You can use kotlin get/set syntax for value
 */
open class KObservableField<T> : ObservableField<T>, Serializable {

    var value: T
        get() = get()
        set(value) { set(value) }

    constructor(init: T) {
        value = init
    }

    constructor(init: T, vararg dependencies: Observable) : super(*dependencies) {
        value = init
    }

    @Suppress("UNCHECKED_CAST")
    override fun get(): T {
        return super.get() as T
    }
}
