package com.topjohnwu.magisk.core.tasks

import androidx.annotation.MainThread

interface FlashResultListener {

    @MainThread
    fun onResult(success: Boolean)

}
