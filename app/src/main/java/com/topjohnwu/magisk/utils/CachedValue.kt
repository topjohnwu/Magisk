package com.topjohnwu.magisk.utils

class CachedValue<T>(private val factory: () -> T) : Lazy<T> {

    private var _val : T? = null

    override val value: T
        get() {
            val local = _val
            return local ?: synchronized(this) {
                val newInstance = factory()
                _val = newInstance
                newInstance
            }
        }

    override fun isInitialized() = _val != null

    fun invalidate() {
        synchronized(this) {
            _val = null
        }
    }
}
