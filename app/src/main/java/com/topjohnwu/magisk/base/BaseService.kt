package com.topjohnwu.magisk.base

import android.app.Service
import android.content.Context
import com.topjohnwu.magisk.utils.wrap
import org.koin.core.KoinComponent

abstract class BaseService : Service(), KoinComponent {
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }
}
