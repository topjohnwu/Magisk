package com.topjohnwu.magisk.ui.base

import android.content.Intent

interface IBaseLeanback {

    fun runWithExternalRW(callback: Runnable)
    fun runWithPermissions(vararg permissions: String, callback: Runnable)
    fun startActivityForResult(intent: Intent, requestCode: Int, listener: ActivityResultListener)

}