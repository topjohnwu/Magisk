package com.topjohnwu.magisk.arch

import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.core.base.ActivityExtension

abstract class UIActivity<T> : AppCompatActivity() {
    abstract val extension: ActivityExtension

    fun withPermission(permission: String, callback: (Boolean) -> Unit) {
        extension.withPermission(permission, callback)
    }

    fun withAuthentication(callback: (Boolean) -> Unit) {
        extension.withAuthentication(callback)
    }
}

