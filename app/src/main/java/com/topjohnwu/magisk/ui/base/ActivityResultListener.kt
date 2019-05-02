package com.topjohnwu.magisk.ui.base

import android.content.Intent

interface ActivityResultListener {
    fun onActivityResult(resultCode: Int, data: Intent?)
}